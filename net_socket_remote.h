#pragma once
#include "net_socket_basic.h"

//协议类型
enum
{
	policy_waitmore,	//等待数据
	policy_bytestream,	//字节流
	policy_websocket,	//websocket,
	policy_http,		//http协议
	policy_flash,		//flash as3
	policy_unknown,		//未知
};

template<class T> struct net_server;
template<class inherit, class socket_service_t>
struct remote_socket_impl : public basic_socket_impl
{
	friend struct net_server<inherit>;
	typedef socket_service_t client_t;

	boost::posix_time::ptime security_check_;
	boost::weak_ptr<client_t>	the_client_;
	std::string					device_id_;
	std::string					client_id_;
	int							id_;
	int							policy_;
	remote_socket_impl(net_server<inherit>& srv) :
		basic_socket_impl(srv.ios_),
		net_server_(srv),
		check_athorize_timer_(srv.ios_),
		check_idle_timer_(srv.ios_)
	{
		authrize_check_time_ = 5000;
		close_when_idle_ = false;
		is_authorized_ = false;
		id_ = 0;
		policy_ = policy_waitmore;
	}

	virtual void	close(bool passive = false)
	{
		basic_socket_impl::close(passive);

		boost::system::error_code ec;
		check_athorize_timer_.cancel(ec);
		check_idle_timer_.cancel(ec);

		auto itf = std::find(net_server_.remotes_.begin(), net_server_.remotes_.end(), shared_from_this());
		if (itf != net_server_.remotes_.end()) {
			net_server_.remotes_.erase(itf);
		}
	}
	std::string	remote_ip()
	{
		boost::system::error_code ec;
		auto edp = s.remote_endpoint(ec);
		return edp.address().to_string(ec);
	}
	unsigned short remote_port()
	{
		boost::system::error_code ec;
		auto edp = s.remote_endpoint(ec);
		return edp.port();
	}
	bool	is_authorized()
	{
		return is_authorized_;
	}

	void	set_authorized()
	{
		is_authorized_ = true;
	}

	void	set_authorize_check(int ms)
	{
		authrize_check_time_ = ms;
	}

protected:
	int				authrize_check_time_;
	boost::asio::deadline_timer check_athorize_timer_, check_idle_timer_;

	bool			is_authorized_;

	net_server<inherit>&		net_server_;

	void	start_recv()
	{
		basic_socket_impl::start_recv();

		check_athorize_timer_.expires_from_now(boost::posix_time::millisec(authrize_check_time_));
		check_athorize_timer_.async_wait(
			boost::bind(&remote_socket_impl<inherit, socket_service_t>::on_check_athorize,
				boost::dynamic_pointer_cast<remote_socket_impl<inherit, socket_service_t>>(shared_from_this()),
				boost::asio::placeholders::error));
	}

	void	on_check_athorize(boost::system::error_code ec)
	{
		if (ec.value() == 0) {
			if (!is_authorized()) {
				close(false);
			}
		}
	}

};

template<class socket_service_t>
class remote_socket : public remote_socket_impl<remote_socket<socket_service_t>, socket_service_t>
{
public:
	remote_socket(net_server<remote_socket<socket_service_t>>& srv) :
		remote_socket_impl<remote_socket<socket_service_t>, socket_service_t>(srv)
	{
        this->set_authorize_check(10000);
	}
};

struct default_socket_service{};

class remote_socket_v2 : public remote_socket_impl<remote_socket_v2, default_socket_service>
{
public:
	int		handling_sequence_;
	remote_socket_v2(net_server<remote_socket_v2>& srv) :
		remote_socket_impl<remote_socket_v2, default_socket_service>(srv)
	{
        this->set_authorize_check(10000);
		handling_sequence_ = 0;
	}
};
