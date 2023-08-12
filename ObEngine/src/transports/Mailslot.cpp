#include "pch.h"
#include "Mailslot.h"


/*
	SERVER
*/
Mailslot::Server::Server(const std::wstring& SlotName)
	: _SlotName(SlotName) {
	
	_FullPath = std::format(L"\\\\.\\mailslot\\{}", SlotName);
}

Mailslot::Server::~Server() {
	if (_hMailslot != INVALID_HANDLE_VALUE)
		::CloseHandle(_hMailslot);
}

/* PRIVATE */
bool Mailslot::Server::SetMailslotAccess() {
	/*
		Create the Everyone SID
	*/
	BYTE worldSid[SECURITY_MAX_SID_SIZE];
	DWORD dwSidSize = sizeof(worldSid);
	auto pWorldSid = (PSID)worldSid;

	if (!::CreateWellKnownSid(WinWorldSid, nullptr, pWorldSid, &dwSidSize)) {
		return false;
	}

	/*
		Get SID of user running this process
	*/
	HANDLE hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		return false;
	}

	BYTE userBuffer[SECURITY_MAX_SID_SIZE + sizeof(TOKEN_USER) +
		sizeof(SID_AND_ATTRIBUTES)];
	auto user = (TOKEN_USER*)userBuffer;
	BOOL ok = ::GetTokenInformation(hToken, TokenUser, userBuffer,
		sizeof(userBuffer), &dwSidSize);
	::CloseHandle(hToken);
	if (!ok) {
		return false;
	}

	auto ownerSid = user->User.Sid;

	/*
		Allocate and initialize a new security descriptor
	*/
	PSECURITY_DESCRIPTOR sd = ::HeapAlloc(::GetProcessHeap(), 0, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (!sd) {
		return false;
	}
	if (!::InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)) {
		::HeapFree(::GetProcessHeap(), 0, sd);
		return false;
	}

	/*
		Build ACEs
	*/
	EXPLICIT_ACCESS ea[2] = { 0 };
	ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (PWSTR)ownerSid;

	ea[1].grfAccessPermissions = FILE_GENERIC_READ;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[1].Trustee.ptstrName = (PWSTR)pWorldSid;

	/*
		Create DACL from ACEs
	*/
	PACL dacl;
	if (ERROR_SUCCESS != ::SetEntriesInAcl(_countof(ea), ea, nullptr, &dacl)) {
		// MS Docs say to only free dacl if successful
		::HeapFree(::GetProcessHeap(), 0, sd);
		return false;
	}

	/*
		Apply the owner and DACL to the security descriptor
	*/
	if (!::SetSecurityDescriptorOwner(sd, ownerSid, FALSE)) {
		::LocalFree(dacl);
		::HeapFree(::GetProcessHeap(), 0, sd);
		return false;
	}

	if (!::SetSecurityDescriptorDacl(sd, TRUE, dacl, FALSE)) {
		::LocalFree(dacl);
		::HeapFree(::GetProcessHeap(), 0, sd);
		return false;
	}

	/*
		Create Mailslot
	*/
	SECURITY_ATTRIBUTES sa = { sizeof(sa) };
	sa.lpSecurityDescriptor = sd;
	_hMailslot = ::CreateMailslotW(_FullPath.c_str(), 1024, MAILSLOT_WAIT_FOREVER, &sa);
	if (_hMailslot == INVALID_HANDLE_VALUE) {
		::LocalFree(dacl);
		::HeapFree(::GetProcessHeap(), 0, sd);
		return false;
	}

	/*
		Cleanup
	*/
	::HeapFree(::GetProcessHeap(), 0, sd);
	::LocalFree(dacl);
	
	return true;
}

/* PUBLIC */
void Mailslot::Server::Start(PTP_SIMPLE_CALLBACK OnMsgRecv) {
	if (!this->SetMailslotAccess()) {
		Helpers::DbgPrint(std::format(L"[Error][Mailslot::Server::Start()]->SetMailslotAccess() - {}",
			_FullPath));
		return;
	}

	// Handle mailslot messages
	DWORD dwNextSize, dwBytesRead;
	BYTE msg[ObEngineRef::MAX_MAILSLOT_MSG_SIZE];

	while (::GetMailslotInfo(_hMailslot, nullptr, &dwNextSize, nullptr, nullptr)) {
		if (dwNextSize == MAILSLOT_NO_MESSAGE) {
			::Sleep(100);
			continue;
		}

		if (!::ReadFile(_hMailslot, &msg, dwNextSize, &dwBytesRead, nullptr)) {
			continue;
		}

		// Verify message size doesn't exceed maximum
		if (dwBytesRead > ObEngineRef::MAX_MAILSLOT_MSG_SIZE) {
			Helpers::DbgPrint(std::format(L"[Error][Mailslot::Server::Start()] Bytes read ({}) exceeds max ({})",
				dwBytesRead, ObEngineRef::MAX_MAILSLOT_MSG_SIZE));
			continue;
		}

		// Verify message is at least the size of a message header
		if (dwBytesRead < sizeof(ObEngineRef::MessageHeader)) {
			Helpers::DbgPrint(std::format(L"[Error][Mailslot::Server::Start()] Bytes read ({}) is less than a message header",
				dwBytesRead));
			continue;
		}

		// Verify number of bytes read matches size reported in message header
		ObEngineRef::MessageHeader* lpMsgHeader = (ObEngineRef::MessageHeader*)msg;
		if (dwBytesRead != sizeof(ObEngineRef::MessageHeader) + lpMsgHeader->Size) {
			Helpers::DbgPrint(std::format(L"[Error][Mailslot::Server::Start()] Bytes read ({}) differs from message header ({})",
				dwBytesRead, sizeof(ObEngineRef::MessageHeader) + lpMsgHeader->Size));
			continue;
		}

		ObEngineRef::MsgRecvContext* lpContext = new ObEngineRef::MsgRecvContext();
		lpContext->lpMessage = (ObEngineRef::MessageHeader*)malloc(dwBytesRead);
		
		if (lpContext->lpMessage == NULL) {
			Helpers::DbgPrint(L"[Error][Mailslot::Server::Start()] Failed to allocate memory for context buffer.");
			continue;
		}
		
		::memcpy(lpContext->lpMessage, lpMsgHeader, dwBytesRead);
		lpContext->dwMsgSize = dwBytesRead;
		
		if (!::TrySubmitThreadpoolCallback(OnMsgRecv, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][Mailslot::Server::Start()] Failed sending message to callback.");
			continue;
		}
	}
}



/*
	CLIENT
*/
Mailslot::Client::Client(const std::wstring& SlotName, const std::wstring& NetBios)
	: _SlotName(SlotName), _NetBios(NetBios) {

	if (NetBios == L".")
		_FullPath = std::format(L"\\\\.\\mailslot\\{}", SlotName);
	else
		_FullPath = std::format(L"\\\\?\\UNC\\{}\\mailslot\\{}", NetBios, SlotName);
}

Mailslot::Client::~Client() {
	if (_hMailslot != INVALID_HANDLE_VALUE)
		::CloseHandle(_hMailslot);
}

/* PUBLIC */
bool Mailslot::Client::Connect() {
	_hMailslot = ::CreateFileW(_FullPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (_hMailslot == INVALID_HANDLE_VALUE) {
		Helpers::DbgPrint(L"[Error][Mailslot::Client::Connect()] Client failed to open mailslot.");
		return false;
	}

	return true;
}

bool Mailslot::Client::Send(const ObEngineRef::MessageHeader& Message, const DWORD dwMsgSize) {
	DWORD dwBytesWrote;

	// Verify message size doesn't exceed maximum
	if (dwMsgSize > ObEngineRef::MAX_MAILSLOT_MSG_SIZE) {
		Helpers::DbgPrint(L"[Error][Mailslot::Client::Send()] Message size exceeds maximum.");
		return false;
	}

	// Verify message is at least the size of a message header
	if (dwMsgSize < sizeof(ObEngineRef::MessageHeader)) {
		Helpers::DbgPrint(L"[Error][Mailslot::Client::Send()] Message size less than header size.");
		return false;
	}

	// Verify number of bytes read matches size reported in message header
	if (dwMsgSize != sizeof(ObEngineRef::MessageHeader) + Message.Size) {
		Helpers::DbgPrint(std::format(L"[Error][Mailslot::Client::Send()] Message size ({}) differs from header ({}).",
			dwMsgSize, Message.Size));
		return false;
	}

	if (!::WriteFile(_hMailslot, &Message, dwMsgSize, &dwBytesWrote, nullptr)) {
		Helpers::DbgPrint(L"[Error][Mailslot::Client::Send()] Client failed to send message.");
		return false;
	}

	return true;
}