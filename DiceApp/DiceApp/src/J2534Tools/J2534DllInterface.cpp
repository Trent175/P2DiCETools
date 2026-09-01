#include "J2534DllInterface.hpp"

void J2534DllInterface::NullAll() {
	PassThruOpen = nullptr;
	PassThruReadMsgs = nullptr;
	PassThruClose = nullptr;
	PassThruConnect = nullptr;
	PassThruDisconnect = nullptr;
	PassThruStartMsgFilter = nullptr;
	PassThruStartPeriodicMsg = nullptr;
	PassThruStopMsgFilter = nullptr;
	PassThruStopPeriodicMsg = nullptr;
	PassThruWriteMsgs = nullptr;
	hModule = nullptr;
}

J2534DllInterface::J2534DllInterface() : m_loaded(false) {
	NullAll();
}

J2534DllInterface::~J2534DllInterface() {
	Unload();
}

template <typename T>
bool LoadFunction(HMODULE hModule, T& funcPtr, LPCSTR exportName) {
	funcPtr = reinterpret_cast<T>(GetProcAddress(hModule, exportName));
	if (!funcPtr) {
		std::cerr << "Failed to locate function " << exportName << std::endl;
		return false;
	}

	return true;
}

int J2534DllInterface::Load(const wchar_t* path) {
	hModule = LoadLibrary(path);

	if (!hModule) {
		std::cerr << "Could not load dll at " << path << std::endl;
		return -1;
	}

	bool success = true;
	success &= LoadFunction(hModule, PassThruOpen, "PassThruOpen");
	success &= LoadFunction(hModule, PassThruReadMsgs, "PassThruReadMsgs");
	success &= LoadFunction(hModule, PassThruClose, "PassThruClose");
	success &= LoadFunction(hModule, PassThruConnect, "PassThruConnect");
	success &= LoadFunction(hModule, PassThruDisconnect, "PassThruDisconnect");
	success &= LoadFunction(hModule, PassThruStartMsgFilter, "PassThruStartMsgFilter");
	success &= LoadFunction(hModule, PassThruStartPeriodicMsg, "PassThruStartPeriodicMsg");
	success &= LoadFunction(hModule, PassThruStopMsgFilter, "PassThruStopMsgFilter");
	success &= LoadFunction(hModule, PassThruStopPeriodicMsg, "PassThruStopPeriodicMsg");
	success &= LoadFunction(hModule, PassThruWriteMsgs, "PassThruWriteMsgs");

	if (!success) {
		Unload();
		return -1;
	}

	m_loaded = true;
	return 0;
}

bool J2534DllInterface::IsLoaded() {
	return m_loaded;
}

void J2534DllInterface::Unload() {
	if (hModule) {
		FreeLibrary(hModule);
	}

	NullAll();
	m_loaded = false;
}

const char* J2534DllInterface::GetErrorCode(J2534_ERROR_CODE err) {
	switch (err) {
		case STATUS_NOERROR:			return "STATUS_NOERROR";
		case ERR_NOT_SUPPORTED:			return "ERR_NOT_SUPPORTED";
		case ERR_INVALID_CHANNEL_ID:	return "ERR_INVALID_CHANNEL_ID";
		case ERR_INVALID_PROTOCOL_ID:	return "ERR_INVALID_PROTOCL_ID";
		case ERR_NULL_PARAMETER:		return "ERR_NULL_PARAMETER";
		case ERR_INVALID_IOCTL_VALUE:	return "ERR_INVALID_IOCTL_VALUE";
		case ERR_INVALID_FLAGS:			return "ERR_INVALID_FLAGS";
		case ERR_FAILED:				return "ERR_FAILED";
		case ERR_DEVICE_NOT_CONNECTED:	return "ERR_DEVICE_NOT_CONNECTED";
		case ERR_TIMEOUT:				return "ERR_TIEMOUT";
		case ERR_INVALID_MSG:			return "ERR_INVALID_MSG";
		case ERR_INVALID_TIME_INTERVAL: return "ERR_INVALID_TIME_INTERVAL";
		case ERR_EXCEEDED_LIMIT:		return "ERR_EXCEEDED_LIMIT";
		case ERR_INVALID_MSG_ID:		return "ERR_INVALID_MSG_ID";
		case ERR_DEVICE_IN_USE:			return "ERR_DEVICE_IN_USE";
		case ERR_INVALID_IOCTL_ID:		return "ERR_INVALID_IOCTL_ID";
		case ERR_BUFFER_EMPTY:			return "ERR_BUFFER_EMPTY";
		case ERR_BUFFER_FULL:			return "ERR_BUFFER_FULL";
		case ERR_BUFFER_OVERFLOW:		return "ERR_BUFFER_OVERFLOW";
		case ERR_PIN_INVALID:			return "ERR_PIN_INVALID";
		case ERR_CHANNEL_IN_USE:		return "ERR_CHANNEL_IN_USE";
		case ERR_MSG_PROTOCOL_ID:		return "ERR_MSG_PROTOCOL_ID";
		case ERR_INVALID_FILTER_ID:		return "ERR_INVALID_FILTER_ID";
		case ERR_NO_FLOW_CONTROL:		return "ERR_NO_FLOW_CONTROL";
		case ERR_NOT_UNIQUE:			return "ERR_NOT_UNIQUE";
		case ERR_INVALID_BAUDRATE:		return "ERR_INVALID_BAUDRATE";
		case ERR_INVALID_DEVICE_ID:		return "ERR_INVALID_DEVICE_ID";
		default:						return "ERR_UNKNOWN";
	}
};