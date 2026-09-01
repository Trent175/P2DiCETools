#include <Windows.h>

#include <iostream>
#include <iomanip>

#include "J2534Tools/J2534Session.hpp"

const wchar_t* dll = L"C:\\Program Files\\DiCE\\Tools\\TSDiCE64.dll";

int main() {
	J2534DllInterface jInterface = J2534DllInterface();
	jInterface.Load(dll);

	J2534Session Session = J2534Session(0, 0, CAN, 500000, jInterface);

	J2534_ERROR_CODE connectError = Session.ConnectDevice(CAN_29BIT_ID);

	if (connectError == STATUS_NOERROR) {
		std::cout << "Connected successfully" << std::endl;
	}
	else {
		std::cout << "Failed to connect " << jInterface.GetErrorCode(connectError) << std::endl;
		return 0;
	}
	
	J2534Data pMask[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	J2534Data pPattern[] = { 0x01, 0x20, 0x00, 0x21 }; // ECM_ME's response ID
	unsigned long filterID = 0;

	J2534_ERROR_CODE filterError = Session.StartMsgFilter(
		PASS_FILTER,
		pMask, pPattern,
		CAN_29BIT_ID,
		filterID);

	if (filterError == STATUS_NOERROR) {
		std::cout << "Filter created, ID: " << filterID << std::endl;
	} else {
		std::cout << "Failed to create filter " << jInterface.GetErrorCode(filterError) << std::endl;
	}

	/*J2534Data pID[] = {0x00, 0x0F, 0xFF, 0xFE}; // 0xFFFFE, big-endian
	J2534Data pFallAsleep[] = { 0xFF, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	J2534Data pWakeUp[] = { 0xFF, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };*/

	// 1. Unregister everything first (clean slate)
	//CanFrame unregisterFrame(0x000FFFFE, { 0xCB, 0x7A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00 }, true);
	CanFrame unregisterFrame = CanFrame::makeD2Request(0x7A, { 0xAA });

	// 2. Register RPM address: nmot_w at 0xF410, size 2
	//    makeRegisterAddrRequest: {0xAA, 0x50}, {addrHigh, addrMid, addrLow, size}
	//CanFrame registerRpm(0x000FFFFE, { 0xCF, 0x7A, 0xAA, 0x50, 0x00, 0xF4, 0x10, 0x02 }, true);
	CanFrame registerRpm = CanFrame::makeD2Request(0x7A, { 0xAA, 0x50 }, { 0x00, 0xF4, 0x10, 0x02 });

	// 3. Poll loop: requestMemory constant = {0xA6, 0xF0, 0x00}, {0x01}
	//CanFrame pollFrame(0x000FFFFE, { 0xCD, 0x7A, 0xA6, 0xF0, 0x00, 0x01, 0x00, 0x00 }, true);
	CanFrame pollFrame = CanFrame::makeD2Request(0x7A, { 0xA6, 0xF0, 0x00 }, { 0x01 });

	//CanFrame enableCommFrame(0x000FFFFE, { 0xCA, 0x50, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00 }, true);
	CanFrame enableCommFrame = CanFrame::makeD2Request(0x50, { 0x50, 0xD8 });
	Session.SendFrame(enableCommFrame);

	Session.SendFrame(unregisterFrame);
	for (int i = 0; i < 3; i++) {
		Session.SendFrame(registerRpm);
	}

	while (true) {
		Session.SendFrame(pollFrame);
		std::vector<PASSTHRU_MSG> msgs;
		J2534_ERROR_CODE readErr = Session.ReadMsgs(msgs, 1, 1000); // short timeout, tight loop
		if (readErr == STATUS_NOERROR) {
			for (PASSTHRU_MSG& msg : msgs) {
				if (msg.DataSize >= 7 && msg.Data[5] == 0x7A && msg.Data[6] == (0xA6 + 0x40)) {
					uint16_t raw = (msg.Data[9] << 8) | msg.Data[10]; // adjust indices once confirmed
					double rpm = raw * 0.25;
					std::cout << "RPM: " << rpm << std::endl;
				}
			}
		}
		else {
			std::cout << jInterface.GetErrorCode(readErr) << std::endl;
		}
		// no Sleep, or a very small one (~10ms) if the bus needs a breather between requests
	}

	return 0;
}