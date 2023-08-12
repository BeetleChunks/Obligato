#pragma once

namespace Helpers {
	std::wstring ErrorMessage();
	std::wstring NtErrorMessage(_In_ const NTSTATUS nStatus);
	std::wstring ToLower(_In_ const std::wstring& data);
	size_t SimpleHashNoCase(_In_ const std::wstring& data);
	std::wstring s2ws(_In_ const std::string& str);
	std::string ws2s(_In_ const std::wstring& wstr);
	void DbgPrint(_In_ const std::wstring& msg);
}