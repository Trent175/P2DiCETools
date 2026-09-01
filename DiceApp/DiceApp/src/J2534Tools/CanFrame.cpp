#include "CanFrame.hpp"

CanFrame::CanFrame(
	uint32_t id,
	std::vector<uint8_t> data,
	bool isExt,
	std::optional<size_t> padLength
) :
	m_id(id),
	m_isExt(isExt)
{
	if (padLength.has_value()) {
		data.resize(*padLength, 0x00); // pad with zeros, or truncate if too long
	}
	this->m_data = std::move(data);
}

PASSTHRU_MSG CanFrame::toPassThruMsg(
	J2534_PROTOCOL protocolId,
	J2534Flags txFlags
) const {
	PASSTHRU_MSG msg{};

	msg.ProtocolID = protocolId;
	msg.TxFlags = txFlags | (m_isExt ? CAN_29BIT_ID : 0);
	msg.Data[0] = static_cast<uint8_t>((m_id >> 24) & 0xFF);
	msg.Data[1] = static_cast<uint8_t>((m_id >> 16) & 0xFF);
	msg.Data[2] = static_cast<uint8_t>((m_id >> 8) & 0xFF);
	msg.Data[3] = static_cast<uint8_t>(m_id & 0xFF);
	memcpy(msg.Data + 4, m_data.data(), m_data.size());
	msg.DataSize = 4 + static_cast<unsigned long>(m_data.size());

	return msg;
}

CanFrame CanFrame::fromPassThruMsg(
	const PASSTHRU_MSG& msg
) {
	uint32_t id = (static_cast<uint32_t>(msg.Data[0]) << 24) |
				  (static_cast<uint32_t>(msg.Data[1]) << 16) |
				  (static_cast<uint32_t>(msg.Data[2]) << 8) |
				   static_cast<uint32_t>(msg.Data[3]);

	std::vector<uint8_t> payload;
	if (msg.DataSize > 4) {
		payload.assign(msg.Data + 4, msg.Data + msg.DataSize);
	}

	bool ext = (msg.RxStatus & CAN_29BIT_ID) != 0;

	return CanFrame(id, std::move(payload), ext);
}


/* static */ CanFrame CanFrame::makeD2Request(
	uint8_t targetId,
	const std::vector<uint8_t>& requestId,
	const std::vector<uint8_t> params,
	bool isExt
) {
	std::vector<uint8_t> logical;
	logical.reserve(1 + requestId.size() + params.size());
	logical.push_back(targetId);
	logical.insert(logical.end(), requestId.begin(), requestId.end());
	logical.insert(logical.end(), params.begin(), params.end());

	if (logical.size() > 7) {
		throw std::runtime_error("makeD2Request: multi-frame D2 requests not yet supported");
	}

	uint8_t totalLength = static_cast<uint8_t>(logical.size());
	uint8_t prefix = 0xC8 + totalLength; // first=1, last=1 -> single frame

	std::vector<uint8_t> payload(8, 0x00);
	payload[0] = prefix;
	std::copy(logical.begin(), logical.end(), payload.begin() + 1);

	return CanFrame(D2_CAN_ID, std::move(payload), true);
}

/* static */ CanFrame CanFrame::makeD2Broadcast(uint8_t targetId, uint8_t command) {
	std::vector<uint8_t> payload(8, 0x00);
	payload[0] = targetId;
	payload[1] = command;
	return CanFrame(D2_CAN_ID, std::move(payload), true);
}

/* static */ D2Response CanFrame::tryParseD2Response(
	const CanFrame& frame,
	uint8_t targetId,
	const std::vector<uint8_t>& requestId
) {
	D2Response result;
	const std::vector<uint8_t>& d = frame.getData();

	if (d.size() < 3) return result; // too short to even have a header

	uint8_t header = d[0];
	if (!(header & 0x80)) return result; // not first frame
	if (d[1] != targetId) return result; // wrong module
	bool errMarker = (d[2] == 0x7F);
	bool ackMarker = (requestId.size() > 0 && d[2] == static_cast<uint8_t>(requestId[0] + 0x40)); // d2 will always be request id 0 + 0x40 ex: 0xA6 + 0x40 = 0xE6
	if (!errMarker && !ackMarker) return result;

	if (!(header & 0x40)) {
		// is multi frame
		return result;
	}

	// single-frame: header = 0xC8 + dataSize
	uint8_t dataSize = header - 0xC8;
	if (d.size() < static_cast<size_t>(1 + dataSize)) return result; // response truncated

	if (errMarker) {
		result.matched = true;
		result.isError = true;
		// [1]=ecuId, [2]=0x7F, [3]=requestId[0] echo, [4]=error code
		result.errorCode = (dataSize >= 4) ? d[4] : 0;
		return result;
	}

	// Validate rest of requestId echo
	size_t echoRegionSize = requestId.size() + 1; // ecuId + requestId
	if (dataSize < echoRegionSize) return result; // response too short to contain full echo

	// Rest of echo will be same as request
	for (size_t i = 1; i < requestId.size(); ++i) {
		if (d[2 + i] != requestId[i]) return result; // echo mismatch, not our frame
	}

	result.matched = true;
	result.payload.assign(d.begin() + 1 + echoRegionSize, d.begin() + 1 + dataSize);
	return result;
}