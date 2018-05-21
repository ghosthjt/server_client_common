#pragma once
#include "net_socket_basic.h"

//Э������
enum
{
	policy_waitmore,	//�ȴ�����
	policy_bytestream,	//�ֽ���
	policy_websocket,	//websocket,
	policy_http,		//httpЭ��
	policy_flash,		//flash as3
	policy_unknown,		//δ֪
};

struct net_server;
class remote_socket_impl : public basic_socket_impl
{
	friend struct net_server;
public:

	boost::posix_time::ptime	security_check_;
	std::string					device_id_;
	std::string					client_id_;
	int							id_;
	int							policy_;
	remote_socket_impl(net_server& srv);

	virtual void	close(bool passive = false);
	std::string	remote_ip();
	unsigned short remote_port();
	bool	is_authorized();

	void	set_authorized();

	void	set_authorize_check(int ms);

protected:
	int				authrize_check_time_;
	boost::asio::deadline_timer check_athorize_timer_, check_idle_timer_;
	bool			is_authorized_;
	net_server&		net_server_;

	void	start_recv();

	void	on_check_athorize(boost::system::error_code ec);

};

class remote_socket_v2 : public remote_socket_impl
{
public:
	int		handling_sequence_;
	remote_socket_v2(net_server& srv);
};
