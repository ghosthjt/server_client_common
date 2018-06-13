#include "http_request.h"
#include "boost/algorithm/string/replace.hpp"
#include "utility.h"
#include "boost/algorithm/string/trim.hpp"

std::string build_http_request(std::string host, std::string param, std::string heads)
{
	std::string fmt = "GET %s HTTP/1.1\r\n";
	char c[1024] = {0};
	sprintf(c, fmt.c_str(), param.c_str());
	std::string ret = c;
	ret += heads;
	ret += "User-Agent:Mozilla/5.0 (Windows NT 10.0; WOW64; rv:46.0) Gecko/20100101 Firefox/46.0\r\n";
	ret += "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	ret += "Accept-Language:zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\n";
	ret += "Accept-Encoding: gzip\r\n";
	ret += "Host:" + host + "\r\n";
	ret += "Connection:Keep-Alive\r\n\r\n";
	return ret;
}

std::map<std::string, std::string> get_http_head_items(const std::string& str_heads, std::string& first_line)
{
	std::map<std::string, std::string> ret;
	std::vector<std::string> sv;
	split_str<std::string>(str_heads, "\r\n", sv, false);

	if (!sv.empty()){
		first_line = sv[0];
	}

	for (int i = 1; i < (int)sv.size(); i++)
	{
		char* p = strstr((char*)sv[i].c_str(), ":");
		if(!p) continue;
		std::string c = p + 1;
		auto it = std::remove(c.begin(), c.end(), ' ');
		if (it != c.end()){
			c.erase(it);
		}
		ret.insert(std::make_pair(std::string((char*)sv[i].c_str(), p), c));
	}
	return ret;
}

void add_http_params(std::string& param, std::string k, long long v)
{
	if (param.empty()){
		param = k + "=" + lx2s(v);
	}
	else 
		param += "&" + k + "=" + lx2s(v);
}

void add_http_params(std::string& param, std::string k, std::string v)
{
	v = HttpUtility::url_encode(v);
	if (param.empty()){
		param = k + "=" + v;
	}
	else 
		param += "&" + k + "=" + v;
}

const char*		http_parse_chunk_begin(const char* recved, int len, int& content_len)
{
	//跳过开始的换行
	const char* lenend = strstr_s(recved, len, "\r\n");
	while (lenend == recved){
		recved += 2;
		lenend = strstr_s(recved, len, "\r\n");
	}

	if (lenend){
		std::string count(recved, lenend);
		if (count == "0"){
			content_len = 0;
			return lenend + 2;
		}
		else{
			sscanf(count.c_str(), "%x", &content_len);
			return lenend + 2;
		}
	}
	return lenend;
}


std::string HttpUtility::url_encode(const std::string &sIn)
{
	std::string sOut;  
	for( size_t ix = 0; ix < sIn.size(); ix++ )  
	{        
		BYTE buf[4];  
		memset( buf, 0, 4 );  
		if( isalnum( (BYTE)sIn[ix] ) )  
		{        
			buf[0] = sIn[ix];  
		}  
		else  
		{  
			buf[0] = '%';  
			buf[1] = tohex( (BYTE)sIn[ix] >> 4 );  
			buf[2] = tohex( (BYTE)sIn[ix] % 16);  
		}   
		sOut += (char *)buf;  
	}  
	return sOut;
}
//codec为 "utf-8", "ucs-2"
std::string HttpUtility::url_decode(const std::string &sIn, std::string codec, bool swap)
{
	std::string sOut;
	bool iswchar = (codec == "ucs-2") || (codec == "ucs2");
	if (iswchar){
		//指示字节顺序
		sOut.push_back(0xFE);
		sOut.push_back(0xFF);
	}
	for( size_t ix = 0; ix < sIn.size(); ix++ )  
	{  
		BYTE ch = 0;
		if(sIn[ix]=='%'){ 
			if ((ix + 5) < sIn.size() && sIn[ix + 1] == 'u' && iswchar){
				if (swap)	{
					ch = (fromhex(sIn[ix + 4]) << 4);  
					ch |= fromhex(sIn[ix + 5]);	
					sOut.push_back(ch);

					ch = (fromhex(sIn[ix + 2]) << 4);  
					ch |= fromhex(sIn[ix + 3]);
					sOut.push_back(ch);

					ix += 5;
				}
				else{
					ch = (fromhex(sIn[ix + 2]) << 4);  
					ch |= fromhex(sIn[ix + 3]);
					sOut.push_back(ch);

					ch = (fromhex(sIn[ix + 4]) << 4);  
					ch |= fromhex(sIn[ix + 5]);	
					sOut.push_back(ch);

					ix += 5;
				}
			}
			else if(ix + 2 < sIn.size()){
				ch = (fromhex(sIn[ix + 1]) << 4);  
				ch |= fromhex(sIn[ix + 2]);  
				ix += 2;
				if (iswchar){
					sOut.push_back(0);
				}
				sOut.push_back(ch);
			}
		}  
		else if(sIn[ix] == '+'){
			if (iswchar){
				sOut.push_back(0);
			}
			sOut.push_back(' ');
		}  
		else {	
			if (iswchar){
				sOut.push_back(0);
			}
			sOut.push_back(sIn[ix]);
		}
	}

	//宽字符调整
	if (iswchar){
		boost::replace_all(sOut, "&nbsp;",		std::string("\0 ",2));
		boost::replace_all(sOut, "&middot;",	std::string("\0·",2));
		boost::replace_all(sOut, "&rsquo;",		std::string("\0'",2));
		boost::replace_all(sOut, "&lsquo;",		std::string("\0'",2));
		boost::replace_all(sOut, "&ldquo;",		std::string("\0\"",2));
		boost::replace_all(sOut, "&rdquo;",	std::string("\0\"",2));
		boost::replace_all(sOut, "&quot;",	std::string("\0\"",2));
		boost::replace_all(sOut, "&mdash;",	std::string("\0—",2));
		boost::replace_all(sOut, "&hellip;",	std::string("\0…",2));
		boost::replace_all(sOut, "&lt;",			std::string("\0<",2));
		boost::replace_all(sOut, "&gt;",			std::string("\0>",2));
		boost::replace_all(sOut, "&rarr;",		std::string("\0→",2));
		boost::replace_all(sOut, "&cap;",			std::string("\0∩",2));
		boost::replace_all(sOut, "&amp;",			std::string("\0&",2));
		sOut.push_back(0);
		sOut.push_back(0);
	}
	else{
		boost::replace_all(sOut, "&nbsp;",		" "		);
		boost::replace_all(sOut, "&middot;",	"·"		);
		boost::replace_all(sOut, "&rsquo;",		"'"		);
		boost::replace_all(sOut, "&lsquo;",		"'"		);
		boost::replace_all(sOut, "&ldquo;",		"\""	);
		boost::replace_all(sOut, "&rdquo;",		"\""	);
		boost::replace_all(sOut, "&quot;",		"\""	);
		boost::replace_all(sOut, "&mdash;",		"—"		);
		boost::replace_all(sOut, "&hellip;",	"……"	);
		boost::replace_all(sOut, "&lt;",			"<"		);
		boost::replace_all(sOut, "&gt;",			">"		);
		boost::replace_all(sOut, "&rarr;",		"→"		);
		boost::replace_all(sOut, "&cap;",			"∩"		);
		boost::replace_all(sOut, "&amp;",			"&"		);
	}
	return sOut;
}

#ifdef WIN32

std::string Unicode2Utf8(const std::wstring& strUtf8)
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

std::string HttpUtility::unescape(const std::string &sIn, std::string to_codec /*= "utf-8"*/)
{
	std::string out;
	for( size_t ix = 0; ix < sIn.size(); ix++ )  
	{  
		BYTE ch = 0;
		//进入%u处理
		if(sIn[ix]=='%' && (ix + 5 < sIn.size()) && sIn[ix + 1] == 'u'){
			std::string tc(sIn.begin() + ix + 2, sIn.begin() + ix + 6);
			wchar_t w = 0;
			sscanf(tc.c_str(), "%x", (unsigned int*)&w);
			std::wstring s; s.push_back(w);
			std::string c = Unicode2Utf8(s);
			out += c;
			ix += 5;
		}
		else{
			out.push_back(sIn[ix]);
		}
	}
	return out;
}

#endif

#include "http_request.h"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/copy.hpp"
#include "file_system.h"

enum
{
	ret_head_report_error = 100,
	ret_content_length_error,
	ret_success,
	ret_continue_recv,
	ret_need_redirect,
};

http_request::http_request(boost::asio::io_service& ios, int init_rdbuffer) : native_socket(ios)
{
	recv_helper_.reading_buffer_ = stream_buffer(boost::shared_array<char>(new char[init_rdbuffer]), 0, init_rdbuffer);
	timeout_ = 0;
	context_.request_state_ = request_state_prepare;
	last_recv_ = 0;
}

void http_request::request(std::string url, std::string extraheader)
{
	str_url_ = url;
	std::string host, port, params;
	split_url(url, host, port, params);
	std::string str_req = build_http_request(host, params, extraheader);
	request(str_req, host, port);
}

int http_request::recv_timeout()
{
	return last_recv_ == 0 ? 0 : (time(nullptr) - last_recv_);
}


void http_request::request(std::string str_request,
	std::string host,
	std::string port)
{
	if (port == "") {
		port = "80";
	}

	str_request_ = str_request;
	str_host_ = host;
	str_port_ = port;
	do_request();
}

void http_request::do_request()
{
	last_recv_ = time(nullptr);

	context_.request_state_ = request_state_connect;

	int ret = async_connect(str_host_.c_str(), s2i<int>(str_port_), [this](int st) {
		if (st == work_state_working) {
			context_.request_state_ = request_state_send_data;
			int ret = send_data(str_request_);
			if (ret != 0) {
				context_.last_error_ = "connected but send request failed.";
				context_.request_state_ = request_state_has_error;
			}
			else {
				context_.request_state_ = request_state_recv_head;
			}
		}
		else {
			context_.last_error_ = "connection failed.";
			context_.request_state_ = request_state_has_error;
		}
	});
}

void http_request::close(bool passive)
{
	native_socket::close(passive);

	if (passive) {
		context_.last_error_ = "connection has been closed.";
		context_.request_state_ = request_state_has_error;
	}
}

bool http_request::is_complete()
{
	return context_.request_state_ == request_state_finished;
}


bool http_request::is_stopped()
{
	return context_.request_state_ >= request_state_stopped_begin &&
		context_.request_state_ < request_state_stopped_end;
}

std::string http_request::ungzip(const std::string& dat)
{
	std::stringstream sdat, sout;
	sdat << dat;

	boost::iostreams::filtering_istream ins;
	ins.push(boost::iostreams::gzip_decompressor());
	ins.push(sdat);
	boost::iostreams::copy(ins, sout);
	return sout.str();
}

int http_request::http_parse_head()
{
	char* recved = recv_helper_.reading_buffer_.data();
	int len = recv_helper_.reading_buffer_.data_left();
	//转化小写
	do {
		char* head_end = (char*)strstr_s(recved, len, "\r\n\r\n");
		if (!head_end) {
			if (recv_helper_.reading_buffer_.data_left() > 1024) {
				context_.last_error_ = "header is too long which is > 1024.";
				return ret_content_length_error;
			}
			else
				return ret_continue_recv;
		}

		//http头改成小区,不改变http体部分
		std::transform(recved, head_end, recved, ::tolower);
		context_.response_header_.insert(context_.response_header_.end(), recved, head_end);

		std::string rsp_code;
		context_.vheads = get_http_head_items(context_.response_header_, rsp_code);

		int rspc = -1;
		if (!rsp_code.empty()) {
			sscanf(rsp_code.c_str(), "http/1.1 %d ", &rspc);
			if (rspc < 0) {
				sscanf(rsp_code.c_str(), "http/1.0 %d ", &rspc);
			}
		}
		else {
			context_.last_error_ = "header error, rsp_code not set.(http/1.1) (http/1.0)";
			return ret_head_report_error;
		}

		if (rspc >= 400) {
			context_.last_error_ = "header error, rspc >= 400";
			return ret_head_report_error;
		}

		if (rspc == 302) {
			auto itf = context_.vheads.find("location");
			if (itf != context_.vheads.end()) {
				context_.redirect_url_ = itf->second;
				return ret_need_redirect;
			}
			else {
				context_.last_error_ = "header response redirect but location is not set.";
				return ret_head_report_error;
			}
		}

		if (rspc != 200 && rspc != 206) {
			context_.last_error_ = "header response status code not support.";
			return ret_content_length_error;
		}

		//得到回复内容长度
		auto itf = context_.vheads.find("content-length");
		if (itf == context_.vheads.end()) {
			itf = context_.vheads.find("transfer-encoding");
			if (itf != context_.vheads.end()) {
				if (itf->second == "chunked") {
					context_.content_t_ = content_t_chunked;
				}
				else {
					context_.last_error_ = "header transfer-encoding is not set. = ";
					return ret_content_length_error;
				}
			}
			else {
				context_.last_error_ = "content-length not set and transfer-encoding not set.";
				return ret_content_length_error;
			}
		}
		else {
			context_.content_t_ = content_t_length;
			context_.content_len_ = 0;
			sscanf(itf->second.c_str(), "%ud", &context_.content_len_);
		}
		//使用数据
		use_recvdata(head_end + 4 - recved);
	} while (0);

	return ret_success;
}

int http_request::on_data_recv(size_t)
{
	last_recv_ = time(nullptr);

	parse();

	if (is_stopped()) {
		return 1;
	}
	else
		return 0;
}

void http_request::parse()
{
	if (context_.request_state_ == request_state_recv_head) {
		int ret = http_parse_head();
		if (ret == ret_success) {
			context_.request_state_ = request_state_recv_body;
		}
		else if (ret == ret_need_redirect) {
			context_.request_state_ = request_state_need_direct;
		}
		else if (ret != ret_continue_recv) {

			context_.request_state_ = request_state_has_error;
		}
	}

	if (context_.request_state_ == request_state_recv_body) {
		if (context_.content_t_ == content_t_length) {
			char* chunk_begin = recv_helper_.reading_buffer_.data();
			int len = recv_helper_.reading_buffer_.data_left();
			context_.response_body_.insert(context_.response_body_.end(), chunk_begin, chunk_begin + len);
			use_recvdata(len);
			//收完
			if (context_.response_body_.size() >= context_.content_len_) {
				context_.request_state_ = request_state_finished;
			}
		}
		else if (context_.content_t_ == content_t_chunked) {
			context_.request_state_ = request_state_chunk_head;
		}
	}

	if (context_.request_state_ == request_state_chunk_head) {
		context_.content_len_ = -1;
		context_.chunk_body_.clear();
		char* chunk_begin = recv_helper_.reading_buffer_.data();
		int len = recv_helper_.reading_buffer_.data_left();

		char* chunk_data = (char*)http_parse_chunk_begin(chunk_begin, len, context_.content_len_);
		//如果找到chunk头了
		if (chunk_data && context_.content_len_ >= 0) {
			//数据接收结束了
			if (context_.content_len_ == 0) {
				context_.request_state_ = request_state_finished;
			}
			else {
				context_.request_state_ = request_state_chunk_data;
				use_recvdata(chunk_data - chunk_begin);
			}
		}
		//如果找不到chunk头,并且字节数已经够多了，则失败
		else if (len > 10) {
			context_.last_error_ = "request_state_chunk_head error, chunk size cannot found.";
			context_.request_state_ = request_state_has_error;
		}
	}

	if (context_.request_state_ == request_state_chunk_data) {
		//如果当前数据多于本chunk数据,则只提取本chunk.
		if ((int)context_.chunk_body_.size() >= context_.content_len_) {
			context_.response_body_.insert(context_.response_body_.end(), context_.chunk_body_.begin(), context_.chunk_body_.end());
			//再处理一次数据,看是不是有其它chunk.
			context_.request_state_ = request_state_chunk_head;
			if (recv_helper_.reading_buffer_.data_left() > 0){
				parse(); return;
			}
		}
		else {
			char* chunk_begin = recv_helper_.reading_buffer_.data();
			int len = recv_helper_.reading_buffer_.data_left();
			//收取chunk体,长度为
			int readl = std::min(len, (context_.content_len_ - (int)context_.chunk_body_.size()));
			context_.chunk_body_.insert(context_.chunk_body_.end(), chunk_begin, chunk_begin + readl);
			use_recvdata(readl);

			if (recv_helper_.reading_buffer_.data_left() > 0){
				parse();	return;
			}
		}
	}

	if (context_.request_state_ == request_state_finished) {
		if(context_.vheads["content-encoding"] == "gzip") {
			context_.response_body_ = ungzip(context_.response_body_);
		}
		recv_body(context_.response_body_);
	}
}

static boost::shared_ptr<http_request_monitor> gins1;
boost::shared_ptr<http_request_monitor> http_request_monitor::get_instance()
{
	if (!gins1) {
		gins1.reset(new http_request_monitor());
	}
	return gins1;
}

int http_request_monitor::stop()
{
	vrequests_.clear();
	return 0;
}

int http_request_monitor::step()
{
	auto it = vrequests_.begin();
	while (it != vrequests_.end())
	{
		monitor_data& md = *it;
		bool will_remove = true;
		//成功了
		if (md.req->is_complete()) {
			md.succ(md.req->context_);
		}
		//失败了
		else if (md.req->is_stopped()) {
			md.failed(md.req->context_);
		}
		//超时
		else if (md.req->recv_timeout() > 5) {
			md.req->context_.request_state_ = request_state_timeout;
			md.failed(md.req->context_);
		}
		else {
			will_remove = false;
		}
		if (will_remove) {
			it = vrequests_.erase(it);
		}
		else
			it++;
	}
	return 0;
}

void http_request_monitor::add_monitor(http_ptr req, std::function<void(http_context&)> succ, std::function<void(http_context&)> failed /*= nullptr*/)
{
	monitor_data md;
	md.req = req;
	md.succ = succ;
	md.failed = failed;
	vrequests_.push_back(md);
}
