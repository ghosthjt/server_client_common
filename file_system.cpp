#include "file_system.h"
#include "utility.h"

typedef int(*wtcb)(fs::path, void*);

int construct_dir_add_file(fs::path  path, void* data)
{
	vdir* pdir = (vdir*) data;
	pdir->file_lst_.push_back(path.string());
	return 0;
}

int construct_dir_add_dir(fs::path  path, void* data)
{
	vdir* pdir = (vdir*) data;
	pdir->dir_lst_.push_back(path.string());
	return 0;
}

int		walk_through_dir(fs::path  path, 
												 wtcb		handle_files, 
												 wtcb		handle_directories,
												 void* data, 
												 bool walk_all = false)
{
	int ret = 0;
	fs::directory_iterator dir_it = fs::directory_iterator(path);
	fs::directory_iterator dir_end;
	while (dir_it != dir_end)
	{
		if (fs::is_regular_file(dir_it->status())){
			if (handle_files)
				ret |= handle_files(dir_it->path(), data);
		}
		else if (fs::is_directory(dir_it->status())){
			if (handle_directories)
				ret |= handle_directories(dir_it->path(), data);

			if (walk_all){
				ret |= walk_through_dir(dir_it->path(), handle_files, handle_directories, data, walk_all);
			}
		}
		dir_it++;
	}
	return ret;
}

int remove_hjtremove(fs::path  path, void* data)
{
	if (strstr(path.string().c_str(),".hjtremove") ||
		strstr(path.string().c_str(),".tmp")){
		boost::system::error_code ec;
		fs::remove(path, ec);
		return ec.value();
	}
	return 0;
}

int clean_rename(std::string dir)
{
	fs::path p(dir);
	return walk_through_dir(p, remove_hjtremove, nullptr, nullptr, true);
}

int remove_dir(std::string dir)
{
	fs::path p(dir);
	boost::system::error_code ec;
	fs::remove_all(p, ec);
	return ec.value() == 0;
}

bool file_exist(std::string f)
{
	fs::path p(f);
	boost::system::error_code ec;
	if (fs::is_regular_file(f, ec)){
		return fs::exists(p);
	}
	else{
		return false;
	}
}

void split_url(const std::string& url, std::string& host, std::string& port, std::string& params)
{
	std::vector<std::string> url_item;
	std::string url_a = url;

	auto it = std::remove(url_a.begin(), url_a.end(), ' ');
	if (it != url_a.end()){
		url_a.erase(it);
	}

	if (strstr(url_a.c_str(), "http://") == url_a.c_str()){
		url_a = url_a.c_str() + strlen("http://");
	}

	if (strstr(url_a.c_str(), "https://") == url_a.c_str()){
		url_a = url_a.c_str() + strlen("https://");
	}

	split_str<std::string>(url_a, "/", url_item, false);
	if (url_item.empty()) return;

	port = "80";
	params = url_a.c_str() + url_item[0].length();

	std::vector<std::string> vhi;
	split_str<std::string>(url_item[0], ":", vhi, false);

	if (vhi.size() == 3){
		host = vhi[1].c_str() + 2; //跳过http:后面的//;
		port = vhi[2];
	}
	else if (vhi.size() == 2){
		host = vhi[0];
		port = vhi[1];
	}
	else if(vhi.size() == 1){
		host = vhi[0];
	}
}

int save_file(std::string name, char* data, unsigned int len, std::string m)
{
	FILE* fp = fopen(name.c_str(), m.c_str());
	if(fp){
		unsigned int wl = 0;
		while (wl < len)
		{
			wl += fwrite(data + wl, 1, len - wl, fp);
		}
		fclose(fp);
		return 0;
	}
	return -1;
}

vdir::vdir(std::string path)
{
	path_ = path;
	collect_dir_info(fs::path(path_));
}

std::string vdir::name()
{
	fs::path p(path_);
	return p.parent_path().filename().string();
}

void vdir::cd(std::string path)
{
	fs::path p(path);
	*this = vdir(p.string());
}

void vdir::cdup()
{
	fs::path p(path_);
	*this = vdir(p.parent_path().string());
}

bool vdir::collect_dir_info(fs::path path)
{
	return walk_through_dir(
		path,
		construct_dir_add_file,
		construct_dir_add_dir,
		this,
		false);
}
