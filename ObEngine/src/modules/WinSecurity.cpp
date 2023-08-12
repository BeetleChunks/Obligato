#include "pch.h"
#include "WinSecurity.h"


/*
	Helper Functions
*/
LSA_HANDLE WinSecurity::GetLocalPolicyHandle() {
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;
	NTSTATUS nStatus;
	LSA_HANDLE lsaPolicyHandle;

	// Object attributes are reserved, so initialize to zeros.
	ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

	// Get a handle to the Policy object.
	nStatus = ::LsaOpenPolicy(
		NULL,              // Use local system
		&ObjectAttributes, // Object attributes.
		POLICY_ALL_ACCESS, // Desired access permissions.
		&lsaPolicyHandle   // Receives the policy handle.
	);

	if (nStatus != STATUS_SUCCESS) {
		// An error occurred. Display it as a win32 error code.
		//std::wcout << Helpers::NtErrorMessage(nStatus) << std::endl;
		return NULL;
	}

	return lsaPolicyHandle;
}

bool WinSecurity::InitLsaString(
	_In_  LPCWSTR pwszString,
	_Out_ PLSA_UNICODE_STRING pLsaString
) {
	*pLsaString = { sizeof(LSA_UNICODE_STRING) };

	size_t sztLen = 0;
	DWORD dwLen = 0;

	if (pLsaString == NULL)
		return false;

	if (pwszString != NULL) {
		sztLen = wcslen(pwszString);
		if (sztLen > 0x7ffe)   // String is too large
			return false;
	}

	// Safely convert size_t to DWORD
	if (::SIZETToDWord(sztLen, &dwLen) != S_OK) {
		return false;
	}

	// Store the string.
	pLsaString->Buffer = (WCHAR*)pwszString;
	pLsaString->Length = (USHORT)dwLen * sizeof(WCHAR);
	pLsaString->MaximumLength = (USHORT)(dwLen + 1) * sizeof(WCHAR);

	return true;
}

PSID WinSecurity::GetLocalAccountSid(
	_In_ LPCWSTR lpcwAccountName
) {
	BOOL bResult;
	PSID pSid;
	SID_NAME_USE enumUse;
	LPWSTR lpwRefDomain;
	DWORD cbSid;
	DWORD cbRefDomain;

	cbSid = cbRefDomain = 0;

	bResult = ::LookupAccountNameW(
		NULL,            // System name - NULL for local
		lpcwAccountName, // Parameter specifying account name
		NULL,            // NULL to get size in 'cbSid'
		&cbSid,          // Count of bytes for account SID buf
		NULL,            // NULL to get size in 'cbSid'
		&cbRefDomain,    // Count of bytes for domain name buf
		&enumUse         // Enumerated use type for account
	);

	// We expect to get an error of ERROR_INSUFFICIENT_BUFFER
	// b/c we set the SID parameter to NULL in order to get
	// the required buffer size via 'cbSid'. This is why the
	// following 'IF' statement is checking for success.
	if (bResult == TRUE) {
		::SetLastError(ERROR_NONE_MAPPED);
		return NULL;
	}

	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return NULL;

	pSid = (PSID)LocalAlloc(LPTR, cbSid);

	if (!pSid)
		return NULL;

	lpwRefDomain = (LPWSTR)LocalAlloc(LPTR, cbRefDomain);

	if (!lpwRefDomain) {
		::LocalFree(pSid);
		return NULL;
	}

	bResult = ::LookupAccountNameW(
		NULL,            // System name - NULL for local
		lpcwAccountName, // Parameter specifying account name
		pSid,            // Pointer to buf that recvs account SID
		&cbSid,          // Count of bytes for account SID buf
		lpwRefDomain,    // Pointer to buf that recvs domain name
		&cbRefDomain,    // Count of bytes for domain name buf
		&enumUse         // Enumerated use type for account
	);

	// We now expect zero errors and the SID
	if (bResult == FALSE) {
		::LocalFree(pSid);
		::LocalFree(lpwRefDomain);
		return NULL;
	}

	::LocalFree(lpwRefDomain);
	return pSid;
}

std::wstring WinSecurity::FromLsaStr(
	_In_ const LSA_UNICODE_STRING& LsaUniStr
) {
	return std::wstring(LsaUniStr.Buffer, LsaUniStr.Length / sizeof(WCHAR));
}

bool WinSecurity::GetUserSessions(
	_In_ const std::wstring& domain,
	_In_ const std::wstring& username,
	_Out_ WinSecurity::Sessions& sessions
) {
	sessions.clear();

	std::wstring TargetDomain = Helpers::ToLower(domain);
	std::wstring TargetUser = Helpers::ToLower(username);

	if (TargetDomain == L".")
		TargetDomain = L"";

	NTSTATUS NtStatus;
	ULONG Count = 0;
	PLUID Sessions;

	NtStatus = ::LsaEnumerateLogonSessions(&Count, &Sessions);
	if (NtStatus != STATUS_SUCCESS) {
		return false;
	}
	
	PSECURITY_LOGON_SESSION_DATA SessionData;
	for (size_t i = 0; i < Count; i++) {
		NtStatus = ::LsaGetLogonSessionData(&Sessions[i], &SessionData);
		if (NtStatus != STATUS_SUCCESS) {
			::LsaFreeReturnBuffer(Sessions);
			return false;
		}

		std::wstring SessUser = Helpers::ToLower(WinSecurity::FromLsaStr(SessionData->UserName));
		std::wstring SessLogonDomain = L"";
		std::wstring SessDnsDomain = L"";

		if (TargetDomain != L"") {
			SessLogonDomain = Helpers::ToLower(WinSecurity::FromLsaStr(SessionData->LogonDomain));
			SessDnsDomain = Helpers::ToLower(WinSecurity::FromLsaStr(SessionData->DnsDomainName));
		}
		
		if ((TargetUser == SessUser) && (TargetDomain == SessLogonDomain || TargetDomain == SessDnsDomain)) {
			WinSecurity::Session CurrentSession;

			CurrentSession[L"AuthPackage"]   = WinSecurity::FromLsaStr(SessionData->AuthenticationPackage);
			CurrentSession[L"DnsDomainName"] = WinSecurity::FromLsaStr(SessionData->DnsDomainName);
			CurrentSession[L"HomeDrive"]     = WinSecurity::FromLsaStr(SessionData->HomeDirectoryDrive);
			CurrentSession[L"HomeFolder"]    = WinSecurity::FromLsaStr(SessionData->HomeDirectory);
			CurrentSession[L"LogonDomain"]   = WinSecurity::FromLsaStr(SessionData->LogonDomain);
			CurrentSession[L"LogonScript"]   = WinSecurity::FromLsaStr(SessionData->LogonScript);
			CurrentSession[L"LogonServer"]   = WinSecurity::FromLsaStr(SessionData->LogonServer);
			CurrentSession[L"LogonType"]     = SessionData->LogonType;
			CurrentSession[L"ProfilePath"]   = WinSecurity::FromLsaStr(SessionData->ProfilePath);
			CurrentSession[L"Session"]       = SessionData->Session;
			CurrentSession[L"Upn"]           = WinSecurity::FromLsaStr(SessionData->Upn);
			CurrentSession[L"UserName"]      = WinSecurity::FromLsaStr(SessionData->UserName);

			sessions.push_back(CurrentSession);
		}
		::LsaFreeReturnBuffer(SessionData);
	}
	::LsaFreeReturnBuffer(Sessions);
	
	return true;
}


/*
	PAM Functions
*/
bool WinSecurity::CreateUser(
	_In_ const std::wstring& user,
	_In_ const std::wstring& password
) {
	NET_API_STATUS nStatus;
	USER_INFO_1 ui;
	DWORD dwLevel = 1;
	DWORD dwError = 0;

	// Need none const
	auto uiUser = user;
	auto uiPass = password;

	// Initialize USER_INFO_1 struct
	ui.usri1_name = &uiUser[0];
	ui.usri1_password = &uiPass[0];
	ui.usri1_priv = USER_PRIV_USER;
	ui.usri1_home_dir = NULL;
	ui.usri1_comment = NULL;
	ui.usri1_flags = UF_SCRIPT;
	ui.usri1_script_path = NULL;

	// Create local user account
	nStatus = ::NetUserAdd(NULL, dwLevel, (LPBYTE)&ui, &dwError);

	if (nStatus == NERR_Success) {
		return true;
	}
	else {
		return false;
	}
}

bool WinSecurity::CreateGroup(
	_In_ const std::wstring& group,
	_In_ const std::wstring& comment
) {
	NET_API_STATUS nStatus;
	LOCALGROUP_INFO_1 lgi;
	DWORD dwLevel = 1;
	DWORD dwError = 0;

	// Need none const
	auto lgiGroup   = group;
	auto lgiComment = comment;

	// Initialize LOCALGROUP_INFO_1 struct
	lgi.lgrpi1_name    = &lgiGroup[0];
	lgi.lgrpi1_comment = &lgiComment[0];

	// Create local user account
	nStatus = ::NetLocalGroupAdd(NULL, dwLevel, (LPBYTE)&lgi, &dwError);

	if (nStatus == NERR_Success) {
		return true;
	}
	else {
		return false;
	}
}


bool WinSecurity::DeleteUser(
	_In_ const std::wstring& user
) {
	NET_API_STATUS nStatus;

	nStatus = ::NetUserDel(NULL, user.c_str());

	if (nStatus == NERR_Success) {
		return true;
	}
	else {
		return false;
	}
}

bool WinSecurity::DeleteGroup(
	_In_ const std::wstring& group
) {
	NET_API_STATUS nStatus;

	nStatus = ::NetLocalGroupDel(NULL, group.c_str());

	if (nStatus == NERR_Success) {
		return true;
	}
	else {
		return false;
	}
}


bool WinSecurity::AddGroupMember(
	_In_ const std::wstring& group,
	_In_ const std::wstring& member
) {
	NET_API_STATUS nStatus;

	// Group data definitions
	LOCALGROUP_MEMBERS_INFO_3 lgmi;
	LOCALGROUP_MEMBERS_INFO_3* lpLgmi = new LOCALGROUP_MEMBERS_INFO_3[1];

	// Need none const
	auto lgiMember = member;

	// Initialize LOCALGROUP_MEMBERS_INFO_3 struct
	lgmi.lgrmi3_domainandname = &lgiMember[0];
	lpLgmi[0] = lgmi;

	// Add member to the local group
	nStatus = ::NetLocalGroupAddMembers(NULL, group.c_str(), 3, (LPBYTE)lpLgmi, 1);

	delete[] lpLgmi;

	if (nStatus == NERR_Success) {
		return true;
	}

	else {
		return false;
	}
}

bool WinSecurity::AddPrivilege(
	_In_ const std::wstring& entity,
	_In_ const std::wstring& privilege
) {
	LSA_HANDLE lsaPolicyHandle;
	LSA_UNICODE_STRING lsaPrivilegeName;
	PSID pSid;
	NTSTATUS ntsResult;

	// Create an LSA_UNICODE_STRING for the privilege names.
	if (!WinSecurity::InitLsaString(privilege.c_str(), &lsaPrivilegeName)) {
		return false;
	}

	// Get a local system policy handle
	lsaPolicyHandle = WinSecurity::GetLocalPolicyHandle();
	if (lsaPolicyHandle == NULL) {
		return false;
	}

	// Get SID for given object name
	pSid = WinSecurity::GetLocalAccountSid(entity.c_str());

	if (pSid == NULL) {
		LsaClose(lsaPolicyHandle);
		return false;
	}

	// Add privilege right to object
	ntsResult = ::LsaAddAccountRights(
		lsaPolicyHandle,	// An open policy handle.
		pSid,				// The target SID.
		&lsaPrivilegeName,	// The privileges.
		1					// Number of privileges.
	);

	// Free the things
	::LsaClose(lsaPolicyHandle);
	::LocalFree(pSid);

	if (ntsResult == STATUS_SUCCESS) {
		return true;
	}
	else {
		return false;
	}
}


bool WinSecurity::RemoveGroupMember(
	_In_ const std::wstring& group,
	_In_ const std::wstring& member
) {
	NET_API_STATUS nStatus;

	// Group data definitions
	LOCALGROUP_MEMBERS_INFO_3 lgmi;
	LOCALGROUP_MEMBERS_INFO_3* lpLgmi = new LOCALGROUP_MEMBERS_INFO_3[1];

	// Need none const
	auto lgiMember = member;

	// Initialize LOCALGROUP_MEMBERS_INFO_3 struct
	lgmi.lgrmi3_domainandname = &lgiMember[0];
	lpLgmi[0] = lgmi;

	// Remove member from the local group
	nStatus = ::NetLocalGroupDelMembers(NULL, group.c_str(), 3, (LPBYTE)lpLgmi, 1);

	delete[] lpLgmi;

	if (nStatus == NERR_Success) {
		return true;
	}

	else {
		return false;
	}
}

bool WinSecurity::RemovePrivilege(
	_In_ const std::wstring& entity,
	_In_ const std::wstring& privilege
) {
	LSA_HANDLE lsaPolicyHandle;
	LSA_UNICODE_STRING lsaPrivilegeName;
	PSID pSid;
	NTSTATUS ntsResult;

	// Create an LSA_UNICODE_STRING for the privilege names.
	if (!WinSecurity::InitLsaString(privilege.c_str(), &lsaPrivilegeName)) {
		return false;
	}

	// Get a local system policy handle
	lsaPolicyHandle = WinSecurity::GetLocalPolicyHandle();
	if (lsaPolicyHandle == NULL) {
		return false;
	}

	// Get SID for given object name
	pSid = WinSecurity::GetLocalAccountSid(entity.c_str());

	if (pSid == NULL) {
		::LsaClose(lsaPolicyHandle);
		return false;
	}

	// Remove privilege right from object
	ntsResult = ::LsaRemoveAccountRights(
		lsaPolicyHandle,	// An open policy handle.
		pSid,				// The target SID.
		FALSE,				// Only remove the specified privilege
		&lsaPrivilegeName,	// The privileges.
		1					// Number of privileges.
	);

	// Free the things
	::LsaClose(lsaPolicyHandle);
	::LocalFree(pSid);

	if (ntsResult == STATUS_SUCCESS) {
		return true;
	}
	else {
		return false;
	}
}


/*
	Session|Process|Token Functions
*/
bool WinSecurity::SessionProcessNameToId(
	_In_  const   std::wstring& ProcName,
	_In_  DWORD   dwSession,
	_Out_ LPDWORD lpProcId
) {
	*lpProcId = 0;

	PWTS_PROCESS_INFOW ProcessInfo = nullptr;
	DWORD dwLevel = WTS_PROCESS_INFO_LEVEL_0;
	DWORD dwCount = 0;

	BOOL bStatus = ::WTSEnumerateProcessesExW(
		WTS_CURRENT_SERVER_HANDLE,
		&dwLevel,
		dwSession,
		(LPWSTR*)&ProcessInfo,
		&dwCount
	);

	if (bStatus == FALSE) {
		::WTSFreeMemoryExW(WTS_TYPE_CLASS::WTSTypeProcessInfoLevel0,
			ProcessInfo, dwCount);
		ProcessInfo = NULL;
		return false;
	}

	auto ProcNameLower = Helpers::ToLower(ProcName);
	for (DWORD i = 0; i < dwCount; i++) {
		std::wstring CurrentName(ProcessInfo[i].pProcessName);

		if (ProcNameLower == Helpers::ToLower(CurrentName)) {
			*lpProcId = ProcessInfo[i].ProcessId;

			::WTSFreeMemoryExW(WTS_TYPE_CLASS::WTSTypeProcessInfoLevel0,
				ProcessInfo, dwCount);
			ProcessInfo = NULL;
			return true;
		}
	}

	// Cleanup
	::WTSFreeMemoryExW(WTS_TYPE_CLASS::WTSTypeProcessInfoLevel0,
		ProcessInfo, dwCount);
	ProcessInfo = NULL;

	// Session process with given name was not found
	return false;
}

bool WinSecurity::EnableTokenPrivilege(
	_In_ HANDLE  hToken,
	_In_ LPCTSTR lpPrivName
) {
	BOOL bResult = FALSE;
	TOKEN_PRIVILEGES tp;
	LUID luid;

	bResult = ::LookupPrivilegeValueW(NULL, lpPrivName, &luid);
	if (bResult != TRUE)
		return false;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bResult = ::AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL
	);
	
	if (bResult != TRUE)
		return false;

	if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		return false;

	return true;
}

bool WinSecurity::SpawnProcessInSession(
	_In_ const std::wstring& app,
	_In_ const std::wstring& args,
	_In_ const std::wstring& cwd,
	_In_ DWORD dwSession,
	_In_ bool bBackground
) {
	// Verify one of 'app' or 'args' are configured
	if (app.empty() && args.empty()) {
		Helpers::DbgPrint(L"[Error][SpawnProcessInSession] Neither 'app' or 'args' is configured.");
		return false;
	}

	// Potential parent process names to use for spoofing
	std::vector<std::wstring> ParentNames = {
		L"explorer.exe",
		L"winlogon.exe"
	};

	// Operation requires our process to have certain privileges enabled
	HANDLE hMyToken = NULL;
	if (::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hMyToken) == FALSE) {
		Helpers::DbgPrint(L"[Error][SpawnProcessInSession] Failed to open handle current process token.");
		return false;
	}

	// Enable SeDebugPrivilege to access user primary tokens
	if (WinSecurity::EnableTokenPrivilege(hMyToken, SE_DEBUG_NAME) == false) {
		Helpers::DbgPrint(L"[Error][SpawnProcessInSession] Failed to enable debug privilege.");
		::CloseHandle(hMyToken);
		return false;
	}

	// Enable SeTcbPrivilege to use CreateProcessAsUser() in all sessions
	if (WinSecurity::EnableTokenPrivilege(hMyToken, SE_TCB_NAME) == false) {
		Helpers::DbgPrint(L"[Error][SpawnProcessInSession] Failed to enable Tcb privilege.");
		::CloseHandle(hMyToken);
		return false;
	}

	if (hMyToken) {
		::CloseHandle(hMyToken);
	}

	// Identify the PID for the surrogate parent process
	bool bParentFound = false;
	DWORD dwParentId = 0;
	for (size_t i = 0; i < ParentNames.size(); i++) {
		bParentFound = WinSecurity::SessionProcessNameToId(ParentNames[i], dwSession, &dwParentId);
		
		if (bParentFound == true) {
			Helpers::DbgPrint(std::format(L"[Debug][SpawnProcessInSession] Found parent pid to spoof: {} ({})",
				ParentNames[i], dwParentId));
			break;
		}
	}

	if (bParentFound == false) {
		Helpers::DbgPrint(L"[Debug][SpawnProcessInSession] Did not find parent pid for spoofing");
		return false;
	}

	// Get handle to surrogate parent 
	HANDLE hParentProc = ::OpenProcess(PROCESS_CREATE_PROCESS, FALSE, dwParentId);
	if (!hParentProc) {
		Helpers::DbgPrint(L"[Debug][SpawnProcessInSession] Failed to open handle to surragate parent process.");
		return false;
	}

	SIZE_T size = 0;
	// Get required attributes size for allocation
	// Error expected: ERROR_INSUFFICIENT_BUFFER
	::InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		Helpers::DbgPrint(std::format(L"[Error][SpawnProcessInSession] Unexpected error for init proc thread attr: {}",
			Helpers::ErrorMessage()));
		::CloseHandle(hParentProc);
		return false;
	}

	// Allocate bufffer with given size and init again
	auto buffer = std::make_unique<BYTE[]>(size);
	auto attrs = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(buffer.get());
	if (::InitializeProcThreadAttributeList(attrs, 1, 0, &size) == FALSE) {
		Helpers::DbgPrint(std::format(L"[Error][SpawnProcessInSession] InitializeProcThreadAttributeList(): {}",
			Helpers::ErrorMessage()));
		::CloseHandle(hParentProc);
		return false;
	}
	if (attrs == 0) {
		Helpers::DbgPrint(L"[Error][SpawnProcessInSession] Attrs is 0 after initialization.");
		::CloseHandle(hParentProc);
		return false;
	}

	// Add parent attribute
	if (::UpdateProcThreadAttribute(attrs, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
		&hParentProc, sizeof(hParentProc), nullptr, nullptr) == FALSE) {

		Helpers::DbgPrint(std::format(L"[Error][SpawnProcessInSession] UpdateProcThreadAttribute(): {}",
			Helpers::ErrorMessage()));
		::CloseHandle(hParentProc);
		return false;
	}

	// Get user's primary logon session token
	HANDLE hUserLogonToken;
	if (::WTSQueryUserToken(dwSession, &hUserLogonToken) == false) {
		Helpers::DbgPrint(std::format(L"[Error][SpawnProcessInSession] WTSQueryUserToken(): {}",
			Helpers::ErrorMessage()));
		::CloseHandle(hParentProc);
		return false;
	}

	// Track success and ensure cleanup is performed
	bool bSuccess = true;
	
	// Default windows station and desktop
	WCHAR DesktopWinSta[] = L"WinSta0\\Default";
	LPVOID lpEnvironment = nullptr;
	DWORD dwCreateFlags = NORMAL_PRIORITY_CLASS | EXTENDED_STARTUPINFO_PRESENT;

	// Control window visability
	if (bBackground == true) {
		dwCreateFlags = dwCreateFlags | CREATE_NO_WINDOW;
	}
	else {
		dwCreateFlags = dwCreateFlags | CREATE_NEW_CONSOLE;
	}

	// Application name; can be null
	LPCWSTR lpAppName = nullptr;
	if (!app.empty())
		lpAppName = app.c_str();

	// Arguments; can be null
	LPWSTR  lpAppArgs = NULL;
	if (!args.empty())
		lpAppArgs = const_cast<WCHAR*>(args.c_str()); // lifetime outlives lpAppArgs

	// Full path for process current working directory; can be NULL
	LPCWSTR lpCurrentDir = NULL;
	if (!cwd.empty())
		lpCurrentDir = cwd.c_str();

	STARTUPINFOEXW siex = { sizeof(siex) };
	PROCESS_INFORMATION ProcInfo;

	ZeroMemory(&ProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siex.StartupInfo, sizeof(STARTUPINFOW));

	siex.StartupInfo.cb = sizeof(STARTUPINFOEXW);
	siex.StartupInfo.lpDesktop = DesktopWinSta;
	siex.lpAttributeList = attrs;

	BOOL bStatus = CreateProcessAsUserW(
		hUserLogonToken,
		lpAppName,
		lpAppArgs,
		NULL,
		NULL,
		FALSE,
		dwCreateFlags,
		lpEnvironment,
		lpCurrentDir,
		(STARTUPINFOW*)&siex,
		&ProcInfo
	);

	if (bStatus == FALSE) {
		Helpers::DbgPrint(std::format(L"[Error][SpawnProcessInSession] CreateProcessAsUser(): {}",
			Helpers::ErrorMessage()));
		bSuccess = false;
	}

	// Cleanup
	if (hParentProc)
		::CloseHandle(hParentProc);
	if (hUserLogonToken)
		::CloseHandle(hUserLogonToken);
	if (ProcInfo.hThread)
		::CloseHandle(ProcInfo.hThread);
	if (ProcInfo.hProcess)
		::CloseHandle(ProcInfo.hProcess);

	::DeleteProcThreadAttributeList(attrs);

	return bSuccess;
}