#pragma once
#include "boost/smart_ptr.hpp"
#include "boost/bind.hpp"
#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/lexical_cast.hpp"
#include <vector>
#include "sending_buffer.h"
#include <list>
#include <iostream>
#include <boost/atomic.hpp>
#include "boost/thread.hpp"
#include "utility.h"

using namespace boost::asio;

typedef	boost::shared_ptr<ip::tcp::acceptor> acceptor_ptr;
typedef boost::shared_ptr<io_service::work>	work_ptr;

enum {
	keep_all,
	keep_new,
	keep_none,
};

enum
{
	work_state_closed,
	work_state_connect_fail,
	work_state_working,
	work_state_connecting,
};

struct send_helper
{
	boost::atomic_bool				is_sending_;
	stream_buffer					sending_buffer_;
	std::list<stream_buffer>		send_vct;//需要进行线程同步
};

struct recv_helper
{
	bool							is_recving_;
	stream_buffer					reading_buffer_;
};

/************************************************************************
	警告:这个类只有发送数据可以多线程调用.
	请保证只在创建线程中调用io_service::poll()或者io_service::run()
************************************************************************/
struct basic_socket_impl : boost::enable_shared_from_this<basic_socket_impl>
{
public:
	recv_helper		recv_helper_;

	basic_socket_impl(boost::asio::io_service& ios);

	virtual ~basic_socket_impl() {}

	virtual void close(bool passive = false);

	void	add_to_send_queue(stream_buffer dat, bool close_this = false);

	int		send_data(stream_buffer dat);

	int		send_data(std::string dat);

	//必须主线程调用
	int		pickup_data(void* out, unsigned int len, bool remove = false, bool all = false);
	void	use_recvdata(unsigned int len) { recv_helper_.reading_buffer_.use_data(len); };
	
	bool	isworking() { return work_state == work_state_working; }
	bool	isconnecting() { return work_state == work_state_connecting; }

    int     last_socket_error() { return last_socket_error_; }

	void	option(std::string name, std::string val) { options_[name] = val; }
	std::string	option(std::string name) { return options_[name]; }
	boost::asio::ip::tcp::endpoint remote_endpoint();
protected:
	ip::tcp::socket                     s;		//要保证同时一间不存在两个线程同时操作socket
    int                                 last_socket_error_;
	boost::atomic_int	                work_state;
	boost::asio::deadline_timer check_data_recv_;
	boost::asio::io_service& ios_;
	boost::atomic_bool	close_when_idle_;	//闲时关闭连接
	send_helper		send_helper_;

	boost::thread::id	create_in_thread_;
	std::map<std::string, std::string> options_;
	std::vector<stream_buffer> pending_send_;
	virtual int		on_data_recv(size_t) {return 0;}
	virtual void	start_recv();

	//切换到创建者线程
	void	switch_to_create_thread(stream_buffer dat, bool close_this, bool sync);

	void	on_recv_check(const boost::system::error_code& ec);

	void	data_recv_handler(const boost::system::error_code& ec, size_t bytes_transferred, bool ischeck);

	bool	send_new_buffer();

	void	on_data_sended_notify(const boost::system::error_code& ec, size_t byte_transfered);

	void	on_data_block_sended(const boost::system::error_code& ec,
		size_t byte_transfered,
		boost::shared_ptr<stream_buffer> pdat,
		boost::shared_ptr<boost::system::error_code> pec);
};
