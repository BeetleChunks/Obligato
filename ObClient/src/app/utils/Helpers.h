#pragma once


namespace Obligato {
	typedef std::basic_string<unsigned char> ustring;

	inline const unsigned char* uc_str(const char* s) {
		return reinterpret_cast<const unsigned char*>(s);
	}

	void Split(const std::string& src, const std::string& delim,
		std::vector<std::string>& results);
	void SplitW(const std::wstring& src, const std::wstring& delim,
		std::vector<std::wstring>& results);

	void RStrip(std::string& src, const std::string& val);
	void RStripW(std::wstring& src, const std::wstring& val);

	void LStrip(std::string& src, const std::string& val);
	void LStripW(std::wstring& src, const std::wstring& val);

	void ReplaceAll(std::string& src, const std::string& from,
		const std::string& to);
	void ReplaceAllW(std::wstring& src, const std::wstring& from,
		const std::wstring& to);

	std::wstring s2ws(const std::string& str);
	std::string ws2s(const std::wstring& wstr);

	void FmtPath(std::string& path);
	void FmtPathW(std::wstring& path);

	void PushPath(std::string& path, const std::string& val);
	void PushPathw(std::wstring& path, const std::wstring& val);

	void PopPath(std::string& path);
	void PopPathW(std::wstring& path);

	void FileFromPath(const std::string& path, std::string& filename);
	void FileFromPathW(const std::wstring& path, std::wstring& filename);

	void ErrorIdToMessage(DWORD errorId, std::wstring& message);
	void ToArgs(const std::string& src, std::vector<std::string>& results);
	std::string to_string(FILETIME ftime);

	std::wstring ToLower(
		_In_ const std::wstring& data
	);
	size_t SimpleHashNoCase(
		_In_ const std::wstring& data
	);
}