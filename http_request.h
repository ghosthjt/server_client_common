#pragma once
#include "boost/smart_ptr.hpp"
#include "boost/asio.hpp"
#include "boost/date_time.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "net_socket_native.h"

using namespace boost::asio;
namespace HttpUtility
{  
	typedef unsigned char BYTE;  

	inline BYTE tohex(const BYTE &x)  
	{  
		return x > 9 ? x -10 + 'A': x + '0';  
	}  

	inline BYTE fromhex(const BYTE &x)  
	{  
		return isdigit(x) ? x-'0' : x-'A'+10;  
	}  

	std::string url_encode(const std::string &sIn);; 
	//codec为 "utf-8", "ucs-2"
	std::string url_decode(const std::string &sIn, std::string codec = "utf-8", bool swap = false);
	//解escape数据,并转成utf-8
	std::string unescape(const std::string &sIn, std::string to_codec = "utf-8");
}

enum 
{
	content_t_length,
	content_t_chunked,
};

enum 
{
	request_state_prepare,
	request_state_connect,
	request_state_send_data,
	request_state_recv_head,
	request_state_recv_body,
	request_state_chunk_head,
	request_state_chunk_data,

	//以下状态表示请示已停止
	request_state_stopped_begin,
	request_state_finished = request_state_stopped_begin,
	request_state_has_error = request_state_stopped_begin + 3,
	request_state_need_direct,
	request_state_timeout,
	request_state_stopped_end,
};

std::string build_http_request(std::string host, std::string param, std::string heads = "");

void add_http_params(std::string& param, std::string k, long long v);

void add_http_params(std::string& param, std::string k, std::string v);

std::map<std::string, std::string> get_http_head_items(const std::string& str_heads, std::string& first_line);

const char*		http_parse_chunk_begin(const char* recved, int len, int& content_len);

struct	http_context
{
	std::string				response_header_, response_body_, chunk_body_;
	int						content_t_, content_len_;
	std::string				redirect_url_;
	int						request_state_;
	std::string				last_error_;
	std::map<std::string, std::string> vheads;
};

class http_request : public native_socket
{
public:
	http_request(boost::asio::io_service& ios, int init_rdbuffer = 1024);
	http_context			context_;

	virtual void			close(bool passive) override;

	void					request(std::string ul, std::string extraheader = "");
	void					request(std::string str_request, std::string host, std::string port);

	bool					is_complete();
	bool					is_stopped();
	std::string				url() {return str_url_;}
	int						recv_timeout();
	
protected:
	std::string				str_url_, str_request_, str_host_, str_port_, extra_header_;
	unsigned int			timeout_;
	time_t					last_recv_;

	virtual int				on_data_recv() override;
	virtual void			recv_body(std::string) {};

	void					parse();
	void					do_request();
	std::string				ungzip(const std::string& dat);
	int						http_parse_head();
};


typedef boost::shared_ptr<http_request> http_ptr;
 
class http_request_monitor
{
public:
	struct monitor_data
	{
		http_ptr req;
		std::function<void(http_context&)> succ;
		std::function<void(http_context&)> failed;
	};
	static		boost::shared_ptr<http_request_monitor> get_instance();
	int			stop();

	int			step();

	void		add_monitor(http_ptr req, std::function<void(http_context&)> succ, std::function<void(http_context&)> failed = nullptr);
protected:
	std::vector<monitor_data> vrequests_;
};
