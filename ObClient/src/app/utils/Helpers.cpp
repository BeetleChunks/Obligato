#include "pch.h"
#include "Helpers.h"

void Obligato::Split(const std::string& src, const std::string& delim,
	std::vector<std::string>& results) {

	std::string current = "";

	for (size_t i = 0; i < src.size(); i++) {
		const char c = src[i];

		if (std::strcmp(&c, delim.c_str()) == 0) {
			if (!current.empty()) {
				results.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}

	if (!current.empty()) {
		results.push_back(current);
	}
}

void Obligato::SplitW(const std::wstring& src, const std::wstring& delim,
	std::vector<std::wstring>& results) {

	std::wstring current = L"";

	for (size_t i = 0; i < src.size(); i++) {
		const wchar_t c = src[i];

		if (std::wcscmp(&c, delim.c_str()) == 0) {
			if (!current.empty()) {
				results.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}

	if (!current.empty()) {
		results.push_back(current);
	}
}


void Obligato::RStrip(std::string& src, const std::string& val) {
	if (src.empty())
		return;
	if (val.empty())
		return;
	if (src.size() < val.size())
		return;

	size_t match_pos = (src.size() - 1);
	while (src.rfind(val, src.size()) == match_pos) {
		src.replace(match_pos, val.length(), "");
		match_pos--;
	}
}

void Obligato::RStripW(std::wstring& src, const std::wstring& val) {
	if (src.empty())
		return;
	if (val.empty())
		return;
	if (src.size() < val.size())
		return;

	size_t match_pos = (src.size() - 1);
	while (src.rfind(val, src.size()) == match_pos) {
		src.replace(match_pos, val.length(), L"");
		match_pos--;
	}
}


void Obligato::LStrip(std::string& src, const std::string& val) {
	if (src.empty())
		return;
	if (val.empty())
		return;
	if (src.size() < val.size())
		return;

	while (src.find(val, 0) == 0) {
		src.replace(0, val.length(), "");
	}
}

void Obligato::LStripW(std::wstring& src, const std::wstring& val) {
	if (src.empty())
		return;
	if (val.empty())
		return;
	if (src.size() < val.size())
		return;

	while (src.find(val, 0) == 0) {
		src.replace(0, val.length(), L"");
	}
}


void Obligato::ReplaceAll(std::string& src, const std::string& from,
	const std::string& to) {

	if (src.empty())
		return;
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = src.find(from, start_pos)) != std::string::npos) {
		src.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

void Obligato::ReplaceAllW(std::wstring& src, const std::wstring& from,
	const std::wstring& to) {

	if (src.empty())
		return;
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = src.find(from, start_pos)) != std::wstring::npos) {
		src.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}


std::wstring Obligato::s2ws(const std::string& str) {
	if (str.empty())
		return std::wstring();

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

	return wstrTo;
}

std::string Obligato::ws2s(const std::wstring& wstr) {
	if (wstr.empty())
		return std::string();

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);

	return strTo;
}


void Obligato::FmtPath(std::string& path) {
	Obligato::LStrip(path, std::string("\\"));
	Obligato::RStrip(path, std::string("\\"));

	path = std::format("\\{}", path);
}

void Obligato::FmtPathW(std::wstring& path) {
	Obligato::LStripW(path, std::wstring(L"\\"));
	Obligato::RStripW(path, std::wstring(L"\\"));

	path = std::format(L"\\{}", path);
}


void Obligato::PushPath(std::string& path, const std::string& val) {
	Obligato::FmtPath(path);
	path = std::format("{}\\{}", path, val);
	Obligato::FmtPath(path);
}

void Obligato::PushPathw(std::wstring& path, const std::wstring& val) {
	Obligato::FmtPathW(path);
	path = std::format(L"{}\\{}", path, val);
	Obligato::FmtPathW(path);
}


void Obligato::PopPath(std::string& path) {
	Obligato::FmtPath(path);

	auto found = path.find_last_of("\\");

	if ((found == std::string::npos) || (found == 0)) {
		path = "\\";
	}
	else {
		path = path.substr(0, found);
	}
}

void Obligato::PopPathW(std::wstring& path) {
	Obligato::FmtPathW(path);

	auto found = path.find_last_of(L"\\");

	if ((found == std::wstring::npos) || (found == 0)) {
		path = L"\\";
	}
	else {
		path = path.substr(0, found);
	}
}


void Obligato::FileFromPath(const std::string& path, std::string& filename) {
	auto found = path.find_last_of("\\");

	if (found == std::string::npos) {
		filename = "";
	}
	else if (path.size() == 1) {
		filename = "";
	}
	else {
		filename = path.substr(found + 1);
	}
}

void Obligato::FileFromPathW(const std::wstring& path, std::wstring& filename) {
	auto found = path.find_last_of(L"\\");

	if (found == std::string::npos) {
		filename = L"";
	}
	else if (path.size() == 1) {
		filename = L"";
	}
	else {
		filename = path.substr(found + 1);
	}
}


void Obligato::ErrorIdToMessage(DWORD errorId, std::wstring& message) {
	LPWSTR lpwErrorText = nullptr;
	DWORD dwCharsCount;

	dwCharsCount = FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpwErrorText,
		0,
		NULL
	);

	std::wstring tmp(lpwErrorText, dwCharsCount);
	message = tmp.c_str();
	LocalFree(lpwErrorText);
}

void Obligato::ToArgs(const std::string& src, std::vector<std::string>& results) {
	LPWSTR* lpwArgsPtr;
	int count;

	// Get required buffer size for conversion
	int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		src.c_str(), -1, nullptr, 0);

	if (size == 0)
		return; // Failed

	// Allocate buffer with required size
	wchar_t* buffer = new wchar_t[size];

	// Convert string to wide char string
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		src.c_str(), -1, buffer, size) == 0) {
		delete[] buffer;
		return;
	}

	std::wstring wstr = buffer;
	delete[] buffer;

	// Parse command to arguments
	lpwArgsPtr = CommandLineToArgvW(wstr.c_str(), &count);

	if (lpwArgsPtr != NULL) {
		for (auto i = 0; i < count; i++) {
			std::wstring wTmp = lpwArgsPtr[i];

			// Get required buffer size for conversion
			auto BufSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				wTmp.c_str(), -1, nullptr, 0, NULL, NULL);

			if (BufSize == 0)
				continue; // Failed

			// Allocate buffer with required size
			char* buffer = new char[BufSize];

			// Convert wide char string to string
			if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				wTmp.c_str(), -1, buffer, BufSize, NULL, NULL) != 0) {

				std::string tmp = buffer;
				results.push_back(tmp);
			}

			delete[] buffer;
		}
	}
}

std::string Obligato::to_string(FILETIME ftime) {
	SYSTEMTIME utc;
	::FileTimeToSystemTime(std::addressof(ftime), std::addressof(utc));

	std::ostringstream stm;
	const auto w2 = std::setw(2);
	stm << std::setfill('0') << std::setw(4) << utc.wYear << '/' << w2 << utc.wMonth
		<< '/' << w2 << utc.wDay << ' ' << w2 << utc.wHour << ':' << w2 << utc.wMinute
		<< ':' << w2 << utc.wSecond << 'Z';

	return stm.str();
}


std::wstring Obligato::ToLower(
	_In_ const std::wstring& data
) {
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

size_t Obligato::SimpleHashNoCase(
	_In_ const std::wstring& data
) {
	std::hash<std::wstring> hasher;
	return hasher(Obligato::ToLower(data));
}