#include "pch.h"
#include "Helpers.h"

namespace Helpers {
	std::wstring ErrorMessage() {
		LPWSTR lpwErrorText = nullptr;
		DWORD dwCharsCount;

		DWORD dwErrorId = ::GetLastError();

		dwCharsCount = ::FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwErrorId,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&lpwErrorText,
			0,
			NULL
		);

		std::wstring message(lpwErrorText, dwCharsCount);
		::LocalFree(lpwErrorText);

		return message;
	}

	std::wstring NtErrorMessage(_In_ const NTSTATUS nStatus) {
		auto error = ::LsaNtStatusToWinError(nStatus);
		return std::wstring(std::format(L"Win32 LSA NT Status Code ({})", error));
	}

	std::wstring ToLower(_In_ const std::wstring& data) {
		auto dataLower = data;

		std::transform(dataLower.begin(), dataLower.end(), dataLower.begin(),
			[](wchar_t c) {
				if (std::isalpha(c))
					return std::tolower(c);
				else
					return (int)c;
			});

		return dataLower;
	}

	size_t SimpleHashNoCase(_In_ const std::wstring& data) {
		std::hash<std::wstring> hasher;
		return hasher(ToLower(data));
	}

	std::wstring s2ws(_In_ const std::string& str) {
		if (str.empty())
			return std::wstring();

		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

		return wstrTo;
	}

	std::string ws2s(_In_ const std::wstring& wstr) {
		if (wstr.empty())
			return std::string();

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);

		return strTo;
	}

	void DbgPrint(_In_ const std::wstring& msg) {
		std::wcout << msg << std::endl;
		::OutputDebugStringW(msg.c_str());
	}
}
