#pragma once
#include "net_socket_basic.h"

class native_socket : public basic_socket_impl
{
public:
	native_socket(boost::asio::io_service& ios);

	static int build_endpoint(std::string ip, int port, boost::asio::ip::tcp::endpoint& edp, bool isv6 = false);

	//成功 0, 其它值，失败
	int		connect( std::string ip, int port,	int time_out = 3 );

	int		connect( const boost::asio::ip::tcp::endpoint & peer_endpoint, int time_out);
	int		async_connect(std::string ip, int port, std::function<void(int)> conn_cb = nullptr);
	
protected:

	ip::tcp::resolver _resolv;
	boost::asio::ip::tcp::resolver::iterator it;
	std::function<void(int)> conn_cb_;
	void connect_handler(const boost::system::error_code& ec);
	void async_connect_handler(const boost::system::error_code& ec);
};

