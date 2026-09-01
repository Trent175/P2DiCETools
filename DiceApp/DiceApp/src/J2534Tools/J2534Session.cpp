#include "J2534Session.hpp"

J2534Session::J2534Session(J2534DeviceID deviceID, J2534ChannelID channelID, J2534_PROTOCOL protocol, unsigned long baudrate, J2534DllInterface& jInterface) :
	m_interface(jInterface),
	m_deviceID(deviceID),
	m_channelID(channelID),
	m_protocol(protocol),
	m_baudrate(baudrate),
	m_connected(false)
{ }

J2534Session::~J2534Session()
{
	// Neatly close the conection
	if (m_connected) {
		m_interface.PassThruDisconnect(m_channelID);
		m_interface.PassThruClose(m_deviceID);
	}
	// unique_ptr delete function is auto called
}

J2534_ERROR_CODE J2534Session::ConnectDevice(
	J2534Flags flags
) {
	if (!m_interface.IsLoaded()) {
		std::cout << "Called ConnectDevice without loading Dll exports!" << std::endl;
		return ERR_FAILED;
	}

	J2534_ERROR_CODE openError = m_interface.PassThruOpen(nullptr, &m_deviceID);

	if (openError != STATUS_NOERROR) {
		return openError;
	}

	J2534_ERROR_CODE connectError = m_interface.PassThruConnect(m_deviceID, m_protocol, flags, m_baudrate, &m_channelID);

	if (connectError == STATUS_NOERROR) {
		m_connected = true;
	}

	return connectError;
}

J2534_ERROR_CODE J2534Session::StartMsgFilter(
	J2534_FILTER filterType,
	J2534Data pMask[],
	J2534Data pPattern[],
	J2534Flags flags,
	unsigned long& filterID
) {
	if (!m_connected) {
		std::cout << "Called StartMsgFilter without connecting device!" << std::endl;
		return ERR_FAILED;
	}

	if (filterType == FLOW_CONTROL_FILTER) {
		std::cout << "Incorrect overload - flow control filter requires pFlow argument" << std::endl;
		return ERR_FAILED;
	}

	PASSTHRU_MSG MaskMsg{}, PatternMsg{};
	MaskMsg.ProtocolID = PatternMsg.ProtocolID = m_protocol;
	MaskMsg.TxFlags = PatternMsg.TxFlags = flags;
	memcpy(&MaskMsg.Data[0], pMask, 4);
	memcpy(&PatternMsg.Data[0], pPattern, 4);
	MaskMsg.DataSize = PatternMsg.DataSize = 4;

	J2534_ERROR_CODE filterError = m_interface.PassThruStartMsgFilter(
		m_channelID,
		filterType,
		&MaskMsg,
		&PatternMsg,
		nullptr, // no flow control for PASS/BLOCK filters
		&filterID
	);

	return filterError;
}

J2534_ERROR_CODE J2534Session::StartMsgFilter(
	J2534_FILTER filterType,
	J2534Data pMask[],
	J2534Data pPattern[],
	J2534Data pFlow[],
	J2534Flags flags,
	unsigned long& filterID
) {
	if (!m_connected) {
		std::cout << "Called StartMsgFilter without connecting device!" << std::endl;
		return ERR_FAILED;
	}

	if (filterType != FLOW_CONTROL_FILTER) {
		std::cout << "Incorrect overload - do not pass non control filter with pFlow argument " << std::endl;
		return ERR_FAILED;
	}

	PASSTHRU_MSG MaskMsg{}, PatternMsg{}, FlowControlMsg{};
	MaskMsg.ProtocolID = PatternMsg.ProtocolID = FlowControlMsg.ProtocolID = m_protocol;
	MaskMsg.TxFlags = PatternMsg.TxFlags = FlowControlMsg.TxFlags = flags;
	memcpy(&MaskMsg.Data[0], pMask, 4);
	memcpy(&PatternMsg.Data[0], pPattern, 4);
	memcpy(&FlowControlMsg.Data[0], pFlow, 4);
	MaskMsg.DataSize = PatternMsg.DataSize = FlowControlMsg.DataSize = 4;

	J2534_ERROR_CODE filterError = m_interface.PassThruStartMsgFilter(
		m_channelID,
		filterType,
		&MaskMsg,
		&PatternMsg,
		&FlowControlMsg,
		&filterID
	);

	return filterError;
}

J2534_ERROR_CODE J2534Session::StopMsgFilter(
	unsigned long filterID
) {
	J2534_ERROR_CODE stopFilterError = m_interface.PassThruStopMsgFilter(m_channelID, filterID);

	return stopFilterError;
}

J2534_ERROR_CODE J2534Session::StartPeriodicMsg(
	J2534Data pID[],
	J2534Data pData[],
	unsigned long dataLen,
	J2534Flags flags,
	unsigned long& msgID,
	unsigned long interval
) {
	PASSTHRU_MSG msg{};
	msg.ProtocolID = m_protocol;
	msg.TxFlags = flags;
	memcpy(&msg.Data[0], pID, 4);
	memcpy(&msg.Data[4], pData, dataLen);
	msg.DataSize = 4 + dataLen;

	J2534_ERROR_CODE startMsgError = m_interface.PassThruStartPeriodicMsg(m_channelID, &msg, &msgID, interval);

	return startMsgError;
}

J2534_ERROR_CODE J2534Session::StopPeriodicMsg(
	unsigned long msgID
) {
	J2534_ERROR_CODE stopMsgError = m_interface.PassThruStopPeriodicMsg(m_channelID, msgID);

	return stopMsgError;
}

J2534_ERROR_CODE J2534Session::ReadMsgs(
	std::vector<PASSTHRU_MSG>& msgs,
	unsigned long maxMsgs,
	unsigned long timeout
) {
	if (!m_connected) {
		std::cout << "Called ReadMsgs without connecting device!" << std::endl;
		return ERR_FAILED;
	}

	msgs.resize(maxMsgs);
	for (PASSTHRU_MSG& m : msgs) { m = PASSTHRU_MSG{}; m.ProtocolID = m_protocol; }

	unsigned long numMsgs = maxMsgs;
	J2534_ERROR_CODE readMsgsError = m_interface.PassThruReadMsgs(
		m_channelID,
		msgs.data(),
		&numMsgs,
		timeout
		);

	msgs.resize(numMsgs);
	return readMsgsError;
}

J2534_ERROR_CODE J2534Session::WriteMsg(
	PASSTHRU_MSG& msg,
	unsigned long timeout
) {
	if (!m_connected) {
		std::cout << "Called WriteMsg without connecting device!" << std::endl;
		return ERR_FAILED;
	}

	unsigned long numMsgs = 1;
	J2534_ERROR_CODE writeError = m_interface.PassThruWriteMsgs(m_channelID, &msg, &numMsgs, timeout);
	
	return writeError;
}

J2534_ERROR_CODE J2534Session::SendFrame(CanFrame& frame, unsigned long timeout) {
	if (!m_connected) {
		std::cout << "Called SendFrame without connecting device!" << std::endl;
		return ERR_FAILED;
	}

	PASSTHRU_MSG msg = frame.toPassThruMsg(m_protocol);
	
	J2534_ERROR_CODE sendError = WriteMsg(msg, timeout);

	return sendError;
}