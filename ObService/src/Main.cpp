#include "pch.h"
#include "ObEngine.h"


thread_local HANDLE tl_hMailslot;
thread_local HANDLE tl_hUdpSocket;

SERVICE_STATUS g_MailslotStatus;
SERVICE_STATUS g_UdpSocketStatus;

HANDLE g_hMailslotStopEvent;
HANDLE g_hUdpSocketStopEvent;

SERVICE_STATUS_HANDLE g_hService;


void SetMailslotStatus(DWORD status);
void WINAPI ObMailslotMain(DWORD dwNumServicesArgs, LPTSTR* args);
DWORD WINAPI ObMailslotHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
DWORD WINAPI StartMailslotThread(PVOID);

void SetUdpSocketStatus(DWORD status);
void WINAPI ObUdpSocketMain(DWORD dwNumServicesArgs, LPTSTR* args);
DWORD WINAPI ObUdpSocketHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
DWORD WINAPI StartUdpSocketThread(PVOID);


int main() {
	/*
		For SERVICE_WIN32_SHARE_PROCESS services, each entry must contain the
		name of a service. This name is the service name that was specified
		by the CreateService function when the service was installed. For
		SERVICE_WIN32_OWN_PROCESS services, the service name in the table
		entry is ignored.

		Because we specify multiple services in the table entry, the service
		when installed must be SERVICE_WIN32_SHARE_PROCESS, otherwise only
		the first service in the table entry structure will execute,
		regardless of the service name as it would be ignored.

		[Install via sc.exe]
		sc create ObMailslot start= auto type= share binPath= "C:\Windows\ObService.exe"
		sc create ObUdpSocket start= auto type= share binPath= "C:\Windows\ObService.exe"
	*/

	WCHAR MailslotSvc[]  = L"ObMailslot";
	WCHAR UdpSocketSvc[] = L"ObUdpSocket";
	
	const SERVICE_TABLE_ENTRY table[] = {
		{ MailslotSvc, ObMailslotMain },
		{ UdpSocketSvc, ObUdpSocketMain },
		{nullptr, nullptr}
	};

	if (!::StartServiceCtrlDispatcher(table))
		return 1;

	return 0;
}


void SetMailslotStatus(DWORD status) {
	g_MailslotStatus.dwCurrentState = status;
	g_MailslotStatus.dwControlsAccepted =
		status == SERVICE_RUNNING ? SERVICE_ACCEPT_STOP : 0;
	::SetServiceStatus(g_hService, &g_MailslotStatus);
}

void WINAPI ObMailslotMain(DWORD dwNumServicesArgs, LPTSTR* args) {
	g_MailslotStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_MailslotStatus.dwWaitHint = 500;

	// Setup security for a mailslot to recv. messages
	bool error = true;
	do {
		// Because this is a "share" type process, name must match the name
		// in the service table entry.
		g_hService = ::RegisterServiceCtrlHandlerEx(L"ObMailslot", ObMailslotHandler, nullptr);
		if (!g_hService)
			break;

		g_hMailslotStopEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!g_hMailslotStopEvent)
			break;

		SetMailslotStatus(SERVICE_START_PENDING);

		// Start mailslot server in a new thread
		tl_hMailslot = ::CreateThread(nullptr, 0, StartMailslotThread, nullptr, 0, nullptr);
		if (tl_hMailslot == NULL)
			break;

		// No errors detected
		error = false;
	
	} while (false);

	// Check if errors were encountered above
	if (error) {
		SetMailslotStatus(SERVICE_STOPPED);
		return;
	}

	SetMailslotStatus(SERVICE_RUNNING);

	// Loop until a service stop request is received
	while (::WaitForSingleObject(g_hMailslotStopEvent, 1000) == WAIT_TIMEOUT) {
		// Verify handler(s) are still active
		if (::WaitForSingleObject(tl_hMailslot, 500) != WAIT_TIMEOUT) {
			break;
		}
	}

	SetMailslotStatus(SERVICE_STOPPED);

	::CloseHandle(g_hMailslotStopEvent);
	::CloseHandle(tl_hMailslot);
}

DWORD WINAPI ObMailslotHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	switch (dwControl) {
	case SERVICE_CONTROL_STOP:
		SetMailslotStatus(SERVICE_STOP_PENDING);
		::SetEvent(g_hMailslotStopEvent);
		break;
	}
	return 0;
}

DWORD WINAPI StartMailslotThread(PVOID) {
	//Helpers::DbgPrint(L"[Info][ObMailslot] Starting Mailslot thread.");
	ObEngine::StartMailslotServer(std::wstring(L"obligato"));
	return 1;
}


void SetUdpSocketStatus(DWORD status) {
	g_UdpSocketStatus.dwCurrentState = status;
	g_UdpSocketStatus.dwControlsAccepted =
		status == SERVICE_RUNNING ? SERVICE_ACCEPT_STOP : 0;
	::SetServiceStatus(g_hService, &g_UdpSocketStatus);
}

void WINAPI ObUdpSocketMain(DWORD dwNumServicesArgs, LPTSTR* args) {
	g_UdpSocketStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_UdpSocketStatus.dwWaitHint = 500;

	// Setup UDP socket to recv. messages
	bool error = true;
	do {
		// Because this is a "share" type process, name must match the name
		// in the service table entry.
		g_hService = ::RegisterServiceCtrlHandlerEx(L"ObUdpSocket", ObUdpSocketHandler, nullptr);
		if (!g_hService)
			break;

		g_hUdpSocketStopEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!g_hUdpSocketStopEvent)
			break;

		SetUdpSocketStatus(SERVICE_START_PENDING);

		// Start UDP socket server in a new thread
		tl_hUdpSocket = ::CreateThread(nullptr, 0, StartUdpSocketThread, nullptr, 0, nullptr);
		if (tl_hUdpSocket == NULL)
			break;

		// No errors detected
		error = false;

	} while (false);

	// Check if errors were encountered above
	if (error) {
		SetUdpSocketStatus(SERVICE_STOPPED);
		return;
	}

	SetUdpSocketStatus(SERVICE_RUNNING);

	// Loop until a service stop request is received
	while (::WaitForSingleObject(g_hUdpSocketStopEvent, 1000) == WAIT_TIMEOUT) {
		// Verify handler(s) are still active
		if (::WaitForSingleObject(tl_hUdpSocket, 500) != WAIT_TIMEOUT) {
			break;
		}
	}

	SetUdpSocketStatus(SERVICE_STOPPED);

	::CloseHandle(g_hUdpSocketStopEvent);
	::CloseHandle(tl_hUdpSocket);
}

DWORD WINAPI ObUdpSocketHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	switch (dwControl) {
	case SERVICE_CONTROL_STOP:
		SetUdpSocketStatus(SERVICE_STOP_PENDING);
		::SetEvent(g_hUdpSocketStopEvent);
		break;
	}
	return 0;
}

DWORD WINAPI StartUdpSocketThread(PVOID) {
	//Helpers::DbgPrint(L"[Info][ObUdpSocket] Starting UDP socket thread.");
	ObEngine::StartUdpSocketServer(std::wstring(L"0.0.0.0"), 49153);
	return 1;
}