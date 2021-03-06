#include <atlconv.h>
#include <atlutil.h>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include "BaseUtil.h"


std::wstring CBaseUtil::Ansi2Unicode(const std::string& strIn)
{
    USES_CONVERSION;
    std::wstring strData = CA2W(strIn.c_str());
    return strData;
}

std::string CBaseUtil::Unicode2Ansi(const std::wstring& strIn)
{
    USES_CONVERSION;
    std::string strData = CW2A(strIn.c_str());
    return strData;
}

std::wstring CBaseUtil::UTF8ToUnicode(const std::string& strUtf8)
{
	int nLen = ::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), strUtf8.length(), NULL, 0);
	
	wchar_t* pszUnicode = new wchar_t[nLen + 1];
	ZeroMemory(pszUnicode, (nLen + 1) * sizeof(wchar_t));

	::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), strUtf8.length(), pszUnicode, nLen);

	std::wstring strUnicode = pszUnicode;
	delete pszUnicode;
	pszUnicode = NULL;

	return strUnicode;
}

std::wstring CBaseUtil::T2Unicode(const tstring& strIn)
{
	USES_CONVERSION;
	std::wstring strData = CT2W(strIn.c_str());
	return strData;
}


tstring CBaseUtil::Ansi2T(const std::string& strIn)
{
	USES_CONVERSION;
	tstring strData = CA2T(strIn.c_str());
	return strData;
}

std::string CBaseUtil::i64toa(__int64 data)
{
	CStringA str;
	str.Format("%I64d", data);
	return (const char*)str;
}

std::string CBaseUtil::UrlEncode(const std::string& strIn)
{
	std::string strUtf8In = Unicode2Utf8(Ansi2Unicode(strIn));

	std::string strOut;

	char szBuffer[1024] = {0};
	DWORD dwLen = _countof(szBuffer) - 1;

	if(!AtlEscapeUrl(strIn.c_str(), szBuffer, &dwLen, _countof(szBuffer) - 1), ATL_URL_ESCAPE)
	{
		char* pszOut = new char[dwLen + 1];
		::memset(pszOut, 0, dwLen + 1);
		if(AtlEscapeUrl(strIn.c_str(), pszOut, &dwLen, dwLen, ATL_URL_ESCAPE))
		{
			strOut = pszOut;
		}

		delete [] pszOut;
		pszOut = NULL;
	}
	else
	{
		strOut = szBuffer;
	}

	return strOut;
}

std::string CBaseUtil::UrlDecode(const std::string& strIn)
{
	std::string strOut;
	char szBuffer[8192] = {0};
	DWORD dwLen = _countof(szBuffer) - 1;
	int n = strIn.length();
	if(!AtlUnescapeUrl(strIn.c_str(), szBuffer, &dwLen, _countof(szBuffer) - 1) )
	{
		dwLen = 8192;
		char* pszOut = /*new char[dwLen + 1]*/szBuffer;
		::memset(pszOut, 0, dwLen + 1);
		if(AtlUnescapeUrl(strIn.c_str(), pszOut, &dwLen, dwLen))
		{
			strOut = pszOut;
		}

		//delete [] pszOut;
		//pszOut = NULL;
	}
	else
	{
		strOut = szBuffer;
	}

	//strOut = Unicode2Ansi(UTF8ToUnicode(strOut));

	return strOut;
}

std::string CBaseUtil::Unicode2Utf8(const std::wstring& strUtf8)
{
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0, NULL, NULL);

	char* pszOut = new char[nLen + 1];
	::memset(pszOut, 0, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, strUtf8.c_str(), strUtf8.size(), pszOut, nLen, NULL, NULL);

	std::string strOut(pszOut);
	delete [] pszOut;
	pszOut = NULL;

	return strOut;
}


long hex2dec(char * s)
{
	int L = strlen(s);
	char c;
	long re = 0;
	while (c = s++[0])
	{
		if (c >='0' && c <='9')
		{
			c -= 48;
		}
		else
		{
			c = c>'Z'? c-32:c;
			c -= 'A'-10;
		}

		re += c*pow(16.0 ,--L);
	}
	return re;
}  

std::string CBaseUtil::Escape(const std::wstring& strIn)
{
	std::string strOut;

	for(int nIndex = 0; nIndex< strIn.size(); ++nIndex)
	{
		if(strIn[nIndex] > 0xff)
		{
			char tmp[5] = {0};
			sprintf(tmp,"%04X",strIn[nIndex]);
			strOut += "%u";
			strOut += tmp;
		}
		else
		{
			if( (strIn[nIndex]>='a' && strIn[nIndex]<='z') 
				|| (strIn[nIndex]>='A' && strIn[nIndex]<='Z') 
				|| ( strIn[nIndex]>='0' && strIn[nIndex]<='9' ) )
			{
				char tmp[2] = {0};
				sprintf(tmp,"%c",strIn[nIndex]);
				strOut += tmp;
			}
			else
			{
				char tmp[3] = {0};
				sprintf(tmp,"%02X",strIn[nIndex]);
				strOut += "%";
				strOut += tmp;
			}
		}
	}

	return strOut;
}

template< class Char_type >
Char_type hex_to_num( const Char_type c )
{
	if( ( c >= '0' ) && ( c <= '9' ) ) return c - '0';
	if( ( c >= 'a' ) && ( c <= 'f' ) ) return c - 'a' + 10;
	if( ( c >= 'A' ) && ( c <= 'F' ) ) return c - 'A' + 10;
	return 0;
}

unsigned int unicode_str_to_char(const std::string& strIn)
{
	return ( hex_to_num( strIn[0] ) << 12 ) + 
		( hex_to_num( strIn[1] ) <<  8 ) + 
		( hex_to_num( strIn[2] ) <<  4 ) + 
		hex_to_num( strIn[3] );
}

std::string CBaseUtil::Unescape(const std::string& strIn)
{
	if (strIn.empty())
	{
		return "";
	}

	// 最后一位字符如果是%s，表明不是escape编码的
	if (strIn[strIn.length() - 1] == '%')
	{
		return strIn;
	}
	
	
	int len = strIn.length() + 1;

	char* encode = new char[len];
	::memset(encode, 0, len);
	::strcpy_s(encode, len, strIn.c_str());

	char* decode = new char[len];
	::memset(decode, 0, len);

	char* temp_decode = decode;
	char* temp_encode = encode;

	std::wstring strUnicodeOut;

	while (temp_encode)
	{
		char* pos = ::strchr(temp_encode,'%');
		if (!pos) 
		{ 
			break; 
		}

		int step = pos - temp_encode;
		if (step)
		{
			memcpy(temp_decode, temp_encode, step);
			strUnicodeOut += UTF8ToUnicode(temp_decode);
			temp_decode += step;
		}

		char code[5] = {0};
		if (pos[1] == 'u')
		{
			memcpy(code, pos + 2, 4);
			temp_encode = pos + 6;
			strUnicodeOut += hex2dec(code);
			memset(code,0,5);
			temp_decode += 2;
		}
		else
		{  
			memcpy(code,pos + 1,2);
			temp_encode = pos + 3;
			temp_decode[0] = hex2dec(code);
			strUnicodeOut += temp_decode[0];
			++temp_decode;
		}
	}
	strcpy_s(temp_decode, strlen(temp_encode) +1, temp_encode);
	strUnicodeOut += UTF8ToUnicode(temp_decode);

	ATLASSERT(_CrtIsValidHeapPointer(decode));
	delete []decode;
	decode = NULL;

	delete []encode;
	encode = NULL;
	
	std::wstring::size_type pos = strUnicodeOut.find(TEXT("\\u"));
	while(pos != std::wstring::npos)
	{
		// 取出\u2039中的数字
		std::wstring strUtf8 = strUnicodeOut.substr(pos + 2, 4);
		if (strUtf8.length() == 4 && IsValidChar(strUtf8))
		{
			std::string strAnsi = Unicode2Ansi(strUtf8);
			WCHAR unicode_char = static_cast<WCHAR>(unicode_str_to_char(strAnsi));
			CString strTemp;
			strTemp.Format(TEXT("%c"), unicode_char);
			strUnicodeOut.replace(pos, 6, (LPCTSTR)strTemp);
		}

		pos = strUnicodeOut.find(TEXT("\\u"), pos + 1);
	}

	std::string strOut = Unicode2Utf8(strUnicodeOut);
	return strOut;
}

std::string CBaseUtil::Encode(const std::wstring& msg)
{
	return CBaseUtil::Escape(msg);
}

std::string CBaseUtil::Decode(const std::string& msg)
{
	// {"的escape编码为%7B%22，以此为标志，判断服务器返回的json字符串是否进行了escape编码
	if (msg.find("%7B%22") != std::string::npos)
	{
		std::string unescape_str = Unescape(msg.c_str());
		return ReplaceJSChar(unescape_str);
	} 
	else
	{
		std::string strIn = msg;
		std::string::size_type pos = strIn.find("\\u");
		while(pos != std::string::npos)
		{
			// 取出\u2039中的数字
			std::string strUtf8 = strIn.substr(pos + 2, 4);
			if (strUtf8.length() == 4 && IsValidChar(strUtf8))
			{
				WCHAR unicode_char = static_cast<WCHAR>(unicode_str_to_char(strUtf8));
				CString strTemp;
				strTemp.Format(TEXT("%c"), unicode_char);

				std::string strUtf8Temp = Unicode2Utf8((LPCTSTR)strTemp);

				strIn.replace(pos, 6, strUtf8Temp);
			}

			pos = strIn.find("\\u", pos + 1);
		}

		return ReplaceJSChar(strIn);
	}
	
	/*std::wstring str = UTF8ToUnicode((const char*)tmp);
	return Unicode2Ansi(UTF8ToUnicode((const char*)tmp));*/
}

std::string CBaseUtil::Decode2(const std::string& msg)
{
	std::string unescape_str = Unescape(msg.c_str());
	return ReplaceJSChar(unescape_str);
}

std::wstring CBaseUtil::DecodeUserName(const std::string& strIn)
{
	//return UTF8ToUnicode(ReplaceJSChar(Unescape(strIn)));

	std::wstring strUnicode = UTF8ToUnicode(strIn);

	CString tmp(strUnicode.c_str());

	tmp.Replace(TEXT("&nbsp;"), TEXT(" "));
	tmp.Replace(TEXT("&middot;"), TEXT("·"));
	tmp.Replace(TEXT("&rsquo;"),TEXT("'"));
	tmp.Replace(TEXT("&lsquo;"),TEXT("'"));
	tmp.Replace(TEXT("&ldquo;"),TEXT("\""));
	tmp.Replace(TEXT("&rdquo;"),TEXT("\""));
	tmp.Replace(TEXT("&quot;"),TEXT("\""));
	tmp.Replace(TEXT("&mdash;"), TEXT("—"));
	tmp.Replace(TEXT("&hellip;"),TEXT("……"));
	tmp.Replace(TEXT("&lt;"),TEXT("<"));
	tmp.Replace(TEXT("&gt;"),TEXT(">"));
	tmp.Replace(TEXT("&rarr;"), TEXT("→"));
	tmp.Replace(TEXT("&cap;"), TEXT("∩"));
	tmp.Replace(TEXT("&amp;"),TEXT("&"));

	return (LPCTSTR)tmp;
}

bool CBaseUtil::IsValidChar(const std::wstring& strIn)
{
	for (std::wstring::size_type index = 0; index < strIn.length(); ++index)
	{
		if (!isxdigit(strIn.at(index)))
		{
			return false;
		}
	}

	return true;
}

bool CBaseUtil::IsValidChar(const std::string& strIn)
{
	for (std::string::size_type index = 0; index < strIn.length(); ++index)
	{
		if (!isxdigit(strIn.at(index)))
		{
			return false;
		}
	}

	return true;
}

std::string CBaseUtil::ReplaceJSChar(const std::string& strIn)
{
	CStringA tmp(strIn.c_str());

	tmp.Replace("&nbsp;", " ");
	tmp.Replace("&middot;", "·");
	tmp.Replace("&rsquo;","'");
	tmp.Replace("&lsquo;","'");
	tmp.Replace("&ldquo;","\"");
	tmp.Replace("&rdquo;","\"");
	tmp.Replace("&quot;","\"");
	tmp.Replace("&mdash;", "—");
	tmp.Replace("&hellip;","……");
	tmp.Replace("&lt;","<");
	tmp.Replace("&gt;",">");
	tmp.Replace("&rarr;", "→");
	tmp.Replace("&cap;", "∩");
	tmp.Replace("&amp;","&");

	return (const char*)tmp;
}
