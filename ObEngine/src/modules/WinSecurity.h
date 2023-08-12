#pragma once

#include "Helpers.h"


namespace WinSecurity {
	using Session = std::unordered_map<std::wstring, std::any>;
	using Sessions = std::vector<Session>;


	/*
		Helper Functions
	*/
	LSA_HANDLE GetLocalPolicyHandle();

	bool InitLsaString(
		_In_  LPCWSTR pwszString,
		_Out_ PLSA_UNICODE_STRING pLsaString
	);
	
	PSID GetLocalAccountSid(
		_In_ LPCWSTR lpcwAccountName
	);
	
	std::wstring FromLsaStr(
		_In_ const LSA_UNICODE_STRING& LsaUniStr
	);
	
	bool GetUserSessions(
		_In_ const std::wstring& domain,
		_In_ const std::wstring& username,
		_Out_ Sessions& sessions
	);

	
	/*
		PAM Functions
	*/
	bool CreateUser(
		_In_ const std::wstring& user,
		_In_ const std::wstring& password
	);
	
	bool CreateGroup(
		_In_ const std::wstring& group,
		_In_ const std::wstring& comment
	);

	
	bool DeleteUser(
		_In_ const std::wstring& user
	);
	
	bool DeleteGroup(
		_In_ const std::wstring& group
	);

	
	bool AddGroupMember(
		_In_ const std::wstring& group,
		_In_ const std::wstring& member
	);
	
	bool AddPrivilege(
		_In_ const std::wstring& entity,
		_In_ const std::wstring& privilege
	);

	
	bool RemoveGroupMember(
		_In_ const std::wstring& group,
		_In_ const std::wstring& member
	);
	
	bool RemovePrivilege(
		_In_ const std::wstring& entity,
		_In_ const std::wstring& privilege
	);

	
	/*
		Session|Process|Token Functions
	*/
	bool SessionProcessNameToId(
		_In_  const   std::wstring& ProcName,
		_In_  DWORD   dwSession,
		_Out_ LPDWORD lpProcId
	);

	bool EnableTokenPrivilege(
		_In_ HANDLE  hToken,
		_In_ LPCTSTR lpPrivName
	);

	bool SpawnProcessInSession(
		_In_ const std::wstring& app,
		_In_ const std::wstring& args,
		_In_ const std::wstring& cwd,
		_In_ DWORD dwSession,
		_In_ bool bBackground
	);
}