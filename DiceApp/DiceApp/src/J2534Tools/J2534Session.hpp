#pragma once

#include <Windows.h>

#include <iostream>
#include <memory>
#include <vector>

#include "J2534DllInterface.hpp"
#include "CanFrame.hpp"

class J2534Session
{
private:
	J2534DllInterface m_interface;
	J2534DeviceID m_deviceID;
	J2534ChannelID m_channelID;
	J2534_PROTOCOL m_protocol;
	unsigned long m_baudrate;
	bool m_connected;
public:
	J2534Session(
		J2534DeviceID deviceID,
		J2534ChannelID channelID,
		J2534_PROTOCOL protocol,
		unsigned long baudrate,
		J2534DllInterface& jInterface
	);
	
	~J2534Session();

	J2534_ERROR_CODE ConnectDevice(
		J2534Flags flags
	);

	J2534_ERROR_CODE StartMsgFilter( // for non flow control
		J2534_FILTER filterType,
		J2534Data pMask[],
		J2534Data pPattern[],
		J2534Flags flags,
		unsigned long& filterID
	);

	J2534_ERROR_CODE StartMsgFilter( // for flow control
		J2534_FILTER filterType,
		J2534Data pMask[],
		J2534Data pPattern[],
		J2534Data pFlow[],
		J2534Flags flags,
		unsigned long& filterID
	);

	J2534_ERROR_CODE StopMsgFilter(
		unsigned long filterID
	);

	// needs rewrite for CanFrame
	J2534_ERROR_CODE StartPeriodicMsg(
		J2534Data pID[],
		J2534Data pData[],
		unsigned long dataLen,
		J2534Flags flags,
		unsigned long& msgID,
		unsigned long interval
	);

	J2534_ERROR_CODE StopPeriodicMsg(
		unsigned long msgID
	);

	J2534_ERROR_CODE ReadMsgs(
		std::vector<PASSTHRU_MSG>& msgs,
		unsigned long maxMsgs = 32,
		unsigned long timeout = 1000
	);

	J2534_ERROR_CODE WriteMsg( // write raw can msg (use send instead)
		PASSTHRU_MSG& msg,
		unsigned long timeout = 1000
	);

	J2534_ERROR_CODE SendFrame(
		CanFrame& frame,
		unsigned long timeout = 1000
	);
};