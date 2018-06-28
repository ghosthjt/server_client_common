#pragma once
#include "boost/lexical_cast.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/thread/locks.hpp"
#include "boost/random.hpp"
#include "boost/locale.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/copy.hpp"
#include <random>
#include <vector>
#include <string>

#define COPY_STR(a, b)\
	strncpy(a, b, sizeof(a) - 1)

static std::string get_guid()
{
	static boost::mutex mutx_;
	boost::lock_guard<boost::mutex> l(mutx_);
	boost::uuids::random_generator uuid_gen;
	boost::uuids::uuid uuid_(uuid_gen());
	return to_string(uuid_);
}

static const char* strstr_s(const char* src, unsigned int len, const char* pat)
{
	std::string str(src, len);
	auto pos = str.find(pat);
	if (pos == str.npos){
		return nullptr;
	}
	return src + pos;
}

static const char* strstr_s(const std::string& str, const char* pat)
{
	auto pos = str.find(pat);
	if (pos == str.npos){
		return nullptr;
	}
	return str.c_str() + pos;
}

static std::string utf8(std::string gb2312)
{
	return boost::locale::conv::to_utf<char>(gb2312, "GB2312");
}

static std::string gbk(std::string u8)
{
	return boost::locale::conv::between(u8, "GB2312", "UTF-8");
}

static std::string ucs2(std::string u8, int enc = 0)
{
	if (enc == 0){
		return boost::locale::conv::to_utf<char>(u8, "GB2312");
	}
	else{
		return boost::locale::conv::to_utf<char>(u8, "UTF-8");
	}
}

static unsigned int rand_r(unsigned int a, unsigned int b)
{
	static std::random_device rd;
	static std::mt19937 mt(rd());                                     
	std::uniform_int_distribution<unsigned int> dist(a, b);
	return dist(mt);
}

static unsigned int rand_r(unsigned int a)
{
	return rand_r(0, a);
}

template<class map_t, class key_t, class value_t>
void replace_map_v(map_t& m, std::pair<key_t, value_t>& val)
{
	auto it = m.find(val.first);
	if (it == m.end()){
		m.insert(val);
	}
	else{
		it->second = val.second;
	}
}
template<class T>
std::string lx2s( T to_cast )
{
	try{
		return boost::lexical_cast<std::string>(to_cast);
	}
	catch(boost::bad_lexical_cast e){
		return "";
	}
}

template<class T>
T s2i( std::string to_cast )
{
	if (to_cast == ""){
		return T();
	}
	try{
		return boost::lexical_cast<T>(to_cast);
	}
	catch(boost::bad_lexical_cast e){
		return T();
	}
}

template<class result_t >
static bool		split_str(std::string src, std::string sep, std::vector<result_t>& res, bool strong_match)
{
	int last_pos = 0;
	int pos = src.find_first_of(sep, 0);
	while (pos != src.npos)
	{
		std::string val(src.c_str() + last_pos, src.c_str() + pos);
        try {
            result_t r = boost::lexical_cast<result_t>(val);
            res.push_back(r);
            last_pos = pos + sep.length();
            pos = src.find_first_of(sep, pos + sep.length());
        }catch(boost::bad_lexical_cast e) {
            // do nothing
        }catch(...) {
            // do nothing
        }
	}

	if (!strong_match){
		std::string val(src.c_str() + last_pos, src.c_str() + src.length());
		if (!val.empty()){
			result_t r = s2i<result_t>(val);
			res.push_back(r);
		}
	}
	return true;
}

template<class T>
std::string		combin_str( T* to_combin, int count)
{
	std::string ret;
	for (int i = 0; i < count; i++)
	{
		ret +=boost::lexical_cast<std::string, T>(to_combin[i]) + ",";
	}
	return ret;
}

template<class T>
std::string		combin_str(std::vector<T> to_combin, std::string sep = ",", bool expand_end = true)
{
	std::string ret;
	for (unsigned int i = 0; i < to_combin.size(); i++)
	{
		if (i == (to_combin.size() - 1)){
			if (expand_end){
				ret += boost::lexical_cast<std::string, T>(to_combin[i]) + sep;
			}
			else
				ret +=boost::lexical_cast<std::string, T>(to_combin[i]);
		}
		else
			ret +=boost::lexical_cast<std::string, T>(to_combin[i]) + sep;
	}
	return ret;
}

static std::string ungzip_gz(const std::string& dat)
{
	try{
		std::stringstream sdat, sout;
		sdat << dat;

		boost::iostreams::filtering_istream ins;
		ins.push(boost::iostreams::gzip_decompressor());
		ins.push(sdat);
		boost::iostreams::copy(ins, sout);
		return sout.str();
	}
	catch(std::exception e){
		return "";
	}
}

static std::string gzip_gz(const std::string& dat)
{
	std::stringstream sdat, sout;
	sdat << dat;

	boost::iostreams::filtering_ostream ins;
	ins.push(boost::iostreams::gzip_compressor());
	ins.push(sout);
	boost::iostreams::copy(sdat, ins);
	return sout.str();
}

class time_counter
{
public:
	time_counter()
	{
		restart();
	}
	void		restart()
	{
		pt = boost::posix_time::microsec_clock::local_time();
	}

	int			elapse()
	{
		return (int)(boost::posix_time::microsec_clock::local_time() - pt).total_milliseconds();
	}
protected:
	boost::posix_time::ptime pt;
};