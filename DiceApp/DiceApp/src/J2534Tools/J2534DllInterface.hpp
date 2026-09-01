#pragma once

#include <Windows.h>

#include <iostream>

#include "J2534DllCommon.hpp"

// function predefinitions

using PassThruOpen_t = J2534_ERROR_CODE(__stdcall*)(
	void* pName,
	J2534DeviceID*deviceID
	);

using PassThruReadMsgs_t = J2534_ERROR_CODE(__stdcall*)(
	J2534ChannelID channelID,
	PASSTHRU_MSG* pMsg,
	unsigned long* pNumMsgs,
	unsigned long timeout
	);

using PassThruClose_t = J2534_ERROR_CODE(__stdcall*)(
	J2534DeviceID deviceID
	);

using PassThruConnect_t = J2534_ERROR_CODE(__stdcall*)(
	J2534DeviceID deviceID,
	J2534_PROTOCOL ProtocolID,
	J2534Flags flags,
	unsigned long BaudRate,
	J2534ChannelID* channelID
	);

using PassThruDisconnect_t = J2534_ERROR_CODE(__stdcall*)(
	J2534DeviceID channelID
	);

using PassThruStartMsgFilter_t = J2534_ERROR_CODE(__stdcall*)(
	J2534ChannelID channelID,
	J2534_FILTER filterType,
	PASSTHRU_MSG* pMaskMsg,
	PASSTHRU_MSG* pPatternMsg,
	PASSTHRU_MSG* pFlowControlMsg,
	unsigned long* pFilterID
	);

using PassThruStartPeriodicMsg_t = J2534_ERROR_CODE(__stdcall*)(
	J2534ChannelID channelID,
	PASSTHRU_MSG* pMsg,
	unsigned long* pMsgID,
	unsigned long TimeInterval
	);

using PassThruStopMsgFilter_t = J2534_ERROR_CODE(__stdcall*)(
	J2534ChannelID channelID,
	unsigned long FilterID
	);

using PassThruStopPeriodicMsg_t = J2534_ERROR_CODE(__stdcall*)(
	J2534ChannelID channelID,
	unsigned long msgID
	);

using PassThruWriteMsgs_t = J2534_ERROR_CODE(__stdcall*)(
	J2534ChannelID channelID,
	PASSTHRU_MSG* pMsg,
	unsigned long* pNumMsgs,
	unsigned long timeout
	);

class J2534DllInterface {
private:
	HMODULE hModule;
	bool m_loaded;
	void NullAll();
public:
	PassThruOpen_t PassThruOpen;
	PassThruReadMsgs_t PassThruReadMsgs;
	PassThruClose_t PassThruClose;
	PassThruConnect_t PassThruConnect;
	PassThruDisconnect_t PassThruDisconnect;
	PassThruStartMsgFilter_t PassThruStartMsgFilter;
	PassThruStartPeriodicMsg_t PassThruStartPeriodicMsg;
	PassThruStopMsgFilter_t PassThruStopMsgFilter;
	PassThruStopPeriodicMsg_t PassThruStopPeriodicMsg;
	PassThruWriteMsgs_t PassThruWriteMsgs;

	J2534DllInterface();
	~J2534DllInterface();

	int Load(const wchar_t* path);
	bool IsLoaded();
	void Unload();
	static const char* GetErrorCode(J2534_ERROR_CODE err);
};