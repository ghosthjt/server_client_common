#pragma once
#include <vector>
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

struct	vdir
{
	std::vector<std::string> file_lst_;
	std::vector<std::string> dir_lst_;

	explicit vdir(std::string path);
	std::string		name();
	void			cd(std::string path);
	void			cdup();
private:
	std::string		path_;
	bool			collect_dir_info(fs::path  path);
};

int					clean_rename(std::string dir);
int					remove_dir(std::string dir);
int					copy_dir(std::string from, std::string to, std::string& err);
bool				file_exist(std::string f);
int					save_file(std::string name, char* data, unsigned int len, std::string m);

void				split_url(const std::string& url, std::string& host, std::string& port, std::string& params);

template<class data_t>
int					save_file(std::string name, data_t& data, std::string m)
{
	FILE* fp = fopen(name.c_str(), m.c_str());
	if(fp){
		unsigned int wl = 0;
		while (wl < data.size())
		{
			wl += fwrite(data.data() + wl, 1, data.size() - wl, fp);
		}
		fclose(fp);
		return 0;
	}
	return -1;
}

template<class ret_t>
int get_file_data(fs::path path, ret_t& ret)
{
	FILE* fp = fopen(path.string().c_str(), "rb");
	if(fp){
		char a[1024];
		int wl = fread(a, 1, 1024, fp);
		while (wl > 0)
		{
			ret.insert(ret.end(), (char*)a, a + wl);
			wl = fread(a, 1, 1024, fp);
		}
		fclose(fp);
		return 0;
	}
	return -1;
}
