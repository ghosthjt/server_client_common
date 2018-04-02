#pragma once

#ifdef _UNICODE
#define  tstring std::wstring
#else
#define tstring std::string
#endif

class  CBaseUtil
{
public:
    static std::wstring Ansi2Unicode(const std::string& strIn);
    static std::string Unicode2Ansi(const std::wstring& strIn);
	static std::wstring UTF8ToUnicode(const std::string& strUtf8);
	static std::string Unicode2Utf8(const std::wstring& strUtf8);
	static std::wstring T2Unicode(const tstring& strIn);
	static tstring Ansi2T(const std::string& strIn);
	static std::string i64toa(__int64 data);
	static std::string UrlEncode(const std::string& strIn);
	static std::string UrlDecode(const std::string& strIn);

	static std::string Escape(const std::wstring& strIn);
	static std::string Unescape(const std::string& strIn);

	static std::string Encode(const std::wstring& msg);
	static std::string Decode(const std::string& msg);
	static std::string Decode2(const std::string& msg);

	static std::wstring DecodeUserName(const std::string& msg);

	static bool IsValidChar(const std::wstring& strIn);

	static bool IsValidChar(const std::string& strIn);

	static std::string ReplaceJSChar(const std::string& strIn);
};

