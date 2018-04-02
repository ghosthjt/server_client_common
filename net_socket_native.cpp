#include "net_socket_native.h"
#include "boost/thread.hpp"

native_socket::native_socket(boost::asio::io_service& ios) :basic_socket_impl(ios), _resolv(ios)
{
	work_state = work_state_closed;
}

int native_socket::build_endpoint(std::string ip, int port, boost::asio::ip::tcp::endpoint& edp, bool isv6 /*= false*/)
{
	boost::system::error_code ec;
	boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip, ec);
	if (ec) {
		return -1;
	}

	if (isv6) {
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v6(), port);
		endpoint.address(addr);
		edp = endpoint;
	}
	else {
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
		endpoint.address(addr);
		edp = endpoint;
	}
	return 0;
}

int native_socket::connect(std::string ip, int port, int time_out /*= 3 */)
{
	boost::system::error_code ec;
	ip::tcp::resolver::query qry(ip, boost::lexical_cast<std::string>(port));
	it = _resolv.resolve(qry, ec);

	while (it != boost::asio::ip::tcp::resolver::iterator())
	{
		if (connect(*it, time_out) == 0) {
			break;
		}
		else {
			close();
		}
		it++;
	}

	if (isworking()) {
		return 0;
	}
	else {
		return -1;
	}
}

int native_socket::connect(const boost::asio::ip::tcp::endpoint & peer_endpoint, int time_out)
{
	work_state = work_state_closed;

	if (time_out > 0) {
		s.async_connect(peer_endpoint, boost::bind(&native_socket::connect_handler,
			boost::dynamic_pointer_cast<native_socket>(shared_from_this()),
			boost::asio::placeholders::error));

		time_t tstart = ::time(nullptr);
		while (::time(nullptr) - tstart < time_out && work_state == work_state_closed)
		{
			ios_.reset();
			ios_.poll();
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		}

		if (isworking()) {
			return 0;
		}
		else
			return -1;
	}
	else {
		s.async_connect(peer_endpoint, boost::bind(&native_socket::async_connect_handler,
			boost::dynamic_pointer_cast<native_socket>(shared_from_this()),
			boost::asio::placeholders::error));
		return 0;
	}
}

int native_socket::async_connect(std::string ip, int port, std::function<void(int)> conn_cb)
{
	boost::system::error_code ec;
	ip::tcp::resolver::query qry(ip, boost::lexical_cast<std::string>(port));
	it = _resolv.resolve(qry, ec);
	conn_cb_ = conn_cb;
	if (it != boost::asio::ip::tcp::resolver::iterator()){
		connect(*it, 0);
		return work_state_connecting;
	}
	else {
		return work_state_connect_fail;
	}
}

void native_socket::connect_handler(const boost::system::error_code& ec)
{
	if (ec.value() == 0) {
		start_recv();
		work_state = work_state_working;
	}
	else {
		work_state = work_state_connect_fail;
	}
}

void native_socket::async_connect_handler(const boost::system::error_code& ec)
{
	if (ec.value() == 0) {
		work_state = work_state_working;
		start_recv();
		if (conn_cb_) conn_cb_(work_state);
	}
	else {
		it++;
		if (it != boost::asio::ip::tcp::resolver::iterator()) {
			connect(*it, 0);
		}
		else {
			if (conn_cb_) conn_cb_(work_state_connect_fail);
		}
	}
}
