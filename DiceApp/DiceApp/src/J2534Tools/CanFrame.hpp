#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <iostream>
#include <cstring>

#include "J2534DllCommon.hpp";

constexpr uint32_t D2_CAN_ID = 0x000FFFFE;

struct D2Response {
	bool matched = false;
	bool isError = false;
	uint8_t errorCode = 0;
	std::vector<uint8_t> payload; // data after the echo region, only valid if matched && !isError
};

/* example return structure 

01 20 00 21 CE 7A E6 F0 00 00 00 00
|_________|	|  V   | |____________|
	 |		| ECU  |	   |
	 V		|______|	   V
Return Addr		V	    Payload
			 Header
*/

class CanFrame {
private:
	uint32_t m_id;
	std::vector<uint8_t> m_data;
	bool m_isExt;
public:
	CanFrame(
		uint32_t id,
		std::vector<uint8_t> data,
		bool isExt = true,
		std::optional<size_t> padLength = std::nullopt
	);

	PASSTHRU_MSG toPassThruMsg(
		J2534_PROTOCOL protocolId,
		J2534Flags txFlags = 0
	) const;

	const std::vector<uint8_t>& getData() const { return m_data; }

	static CanFrame fromPassThruMsg(
		const PASSTHRU_MSG& msg
	);

	static CanFrame makeD2Request(
		uint8_t targetId,
		const std::vector<uint8_t>& requestId,
		const std::vector<uint8_t> params = {},
		bool isExt = true
	);

	static CanFrame makeD2Broadcast(
		uint8_t targetId,
		uint8_t command
	);

	static D2Response tryParseD2Response(
		const CanFrame& frame,
		uint8_t targetId,
		const std::vector<uint8_t>& requestId
	);
};