#include "net_socket_remote.h"
#include "net_service.h"

remote_socket_impl::remote_socket_impl(net_server& srv) :
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

void remote_socket_impl::close(bool passive /*= false*/)
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

std::string remote_socket_impl::remote_ip()
{
	boost::system::error_code ec;
	auto edp = s.remote_endpoint(ec);
	return edp.address().to_string(ec);
}

unsigned short remote_socket_impl::remote_port()
{
	boost::system::error_code ec;
	auto edp = s.remote_endpoint(ec);
	return edp.port();
}

bool remote_socket_impl::is_authorized()
{
	return is_authorized_;
}

void remote_socket_impl::set_authorized()
{
	is_authorized_ = true;
}

void remote_socket_impl::set_authorize_check(int ms)
{
	authrize_check_time_ = ms;
}

void remote_socket_impl::start_recv()
{
	basic_socket_impl::start_recv();

	check_athorize_timer_.expires_from_now(boost::posix_time::millisec(authrize_check_time_));
	check_athorize_timer_.async_wait(
		boost::bind(&remote_socket_impl::on_check_athorize,
			boost::dynamic_pointer_cast<remote_socket_impl>(shared_from_this()),
			boost::asio::placeholders::error));
}

void remote_socket_impl::on_check_athorize(boost::system::error_code ec)
{
	if (ec.value() == 0) {
		if (!is_authorized()) {
			close(false);
		}
	}
}

remote_socket_v2::remote_socket_v2(net_server& srv) : remote_socket_impl(srv)
{
	this->set_authorize_check(10000);
	handling_sequence_ = 0;
}
