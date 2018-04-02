#pragma once
#include <string>

void load_ban_word(std::string path);

bool check_ban_word(std::string utf8_str_in, std::string & utf8_str_out);
