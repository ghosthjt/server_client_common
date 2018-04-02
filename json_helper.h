#pragma once
#include <iosfwd>
#include <string>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "utility.h"

struct stream_base;

class json_msg_helper
{
public:
	static bool from_json(stream_base* msg, const char* json_string);
	static std::string to_json(stream_base* p);

	template<class T>
	static T read_value(boost::property_tree::ptree& src, std::string key, T def)
	{
		if (src.find(key) != src.not_found()) {
			std::string ss = src.get<std::string>(key);
			if (!ss.empty()) {
				return s2i<T>(ss);
			}
			else {
				return def;
			}
		}
		return def;
	}

	template<class T> 
	static void read_value(std::string field, T& val, boost::property_tree::ptree& jsvalue)
	{
		val = read_value(jsvalue, field, val);
	}

	template<class T>
	static void read_arr_value(std::string field, T* arr, unsigned int arr_size, boost::property_tree::ptree& jsvalue)
	{
		std::string strv = read_value<std::string>(jsvalue, field, "");
		std::vector<T> v;
		split_str(strv, ",", v, true);
		unsigned int i = 0;
		for ( ; i < v.size() && i < arr_size; i++)
		{
			arr[i] = v[i];
		}
	}

	static void read_arr_value(std::string field, char* arr, unsigned int arr_size, boost::property_tree::ptree& jsvalue)
	{
		std::string strv = read_value<std::string>(jsvalue, field, "");
		strncpy(arr, strv.c_str(), arr_size - 1);
	}

	template<class T>
	static void read_vec_value(std::string field, std::vector<T>& arr, boost::property_tree::ptree& jsvalue)
	{
		std::string strv = read_value<std::string>(jsvalue, field, "");
		split_str<T>(strv, ",", arr, true);
	}

	template<class T>
	static void write_value(std::string field, T val, boost::property_tree::ptree& jsvalue)
	{
		jsvalue.put<T>(field, val);
	}

	template<class T>
	static void write_arr_value(std::string field, T* arr, int arr_size, boost::property_tree::ptree& jsvalue)
	{
		std::string strv = combin_str<T>(arr, arr_size);
		jsvalue.put<std::string>(field, strv);
	}

	template<class T>
	static void write_arr_value(std::string field, std::vector<T> arr, boost::property_tree::ptree& jsvalue)
	{
		std::string strv = combin_str<T>(arr);
		jsvalue.put<std::string>(field, strv);
	}

	template<class T>
	static void write_vector_to_string(std::string field, std::vector<T> vec, boost::property_tree::ptree& jsvalue)
	{
		std::string ret;
		size_t len = vec.size();
		for (size_t i = 0; i < len; i++) {
			ret += s2i<T>(vec[i]) + ",";
		}
		jsvalue.put<std::string>(field, ret);
	}
};

#define read_jvalue(field, jsval)	 json_msg_helper::read_value(#field, field, jsval)
#define write_jvalue(field, jsval)	 json_msg_helper::write_value(#field, field, jsval)
#define read_jstring(field, sz, jsval)	 json_msg_helper::read_arr_value(#field, field, sz, jsval)

#define read_jarr(field, sz, jsval)	 json_msg_helper::read_arr_value(#field, field, sz, jsval)
#define read_jvec(field, jsval)	 json_msg_helper::read_vec_value(#field, field, jsval)

#define write_jarr(field, sz, jsval) json_msg_helper::write_arr_value(#field, field, sz, jsval)
#define write_jvec(field, jsval)	 json_msg_helper::write_arr_value(#field, field, jsval)
//改成和上面一样的
//#define write_vector_value(field, jsval) json_msg_helper::write_vector_to_string(#field, field, jsval)