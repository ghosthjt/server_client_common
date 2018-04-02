#include "ban_word.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <locale>
#include <wtypes.h>
#include <vector>
#include <map>
#include <list>
#include <fstream>
#include <boost/locale.hpp>

using namespace std;

bool is_load = false;
std::map<WCHAR, std::list<std::wstring>> glb_map_ban_word;

void load_ban_word(std::string path)
{
	std::vector<std::wstring> ban_word;

	wifstream in(path);
	in.imbue(std::locale("chs"));
	if (!in.is_open()) {
		std::wcout << "open file error" << endl;
	}

	while (!in.eof())
	{
		WCHAR str[100] = { 0 };
		in.getline(str, 100);
		if (wcslen(str) > 0) {
			ban_word.push_back(str);
		}
	}
	
	glb_map_ban_word.clear();
	for (unsigned i = 0; i < ban_word.size(); i++)
	{
		auto & it = glb_map_ban_word[ban_word[i][0]];
		it.push_back(ban_word[i]);
	}

	for (auto it = glb_map_ban_word.begin(); it != glb_map_ban_word.end(); it++)
	{
		auto & it2 = it->second;
		it2.sort([&](std::wstring & a, std::wstring & b)->bool {return a.size() > b.size(); });
	}

	is_load = true;
}

bool check_ban_word(std::string utf8_str_in, std::string & utf8_str_out)
{
	if (is_load == false) {
		load_ban_word("./ban_word.txt");
	}

	wcout.imbue(std::locale("chs"));
	bool is_find = false;
	std::wstring wstr_in = boost::locale::conv::utf_to_utf<wchar_t>(utf8_str_in);

	for (unsigned i = 0; i < wstr_in.size(); i++)
	{
		wchar_t* p = &wstr_in[i];
		auto it = glb_map_ban_word.find(*p);
		if (it == glb_map_ban_word.end()) {
			continue;
		}

		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			if (wstr_in.length() - i >= (*it2).length()) {
				int val = _wcsnicmp(p, (*it2).c_str(), (*it2).length());
				if (0 == val) {
					is_find = true;
					wstr_in.replace(i, (*it2).size(), (*it2).size(), '*');
					i += ((*it2).size() - 1);
					break;
				}
			}
		}
	}

	utf8_str_out = boost::locale::conv::utf_to_utf<char>(wstr_in);
	return is_find;
}
