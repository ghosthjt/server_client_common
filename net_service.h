#pragma once
#include "net_socket_basic.h"
#include "net_socket_remote.h"

template<class remote_t>
struct net_server
{
	typedef boost::shared_ptr<remote_t> remote_socket_ptr;
	friend struct remote_socket_impl<remote_t, typename remote_t::client_t>;
protected:
	std::vector<acceptor_ptr>	  acceptors_;
	std::vector<unsigned short>	ports_;
	bool												stoped_;
	std::vector<remote_socket_ptr>	remotes_;

public:
	io_service			ios_;
	explicit net_server()
	{
	}

	void	add_acceptor(unsigned short port)
	{
		ports_.push_back(port);
	}

	bool	run()
	{
		boost::system::error_code ec;
		stoped_ = false;
		ios_.reset();
		for (size_t i = 0; i < ports_.size(); i++)
		{
			acceptors_.push_back(acceptor_ptr(new ip::tcp::acceptor(ios_)));
		}

		for (size_t i = 0; i < acceptors_.size(); i++)
		{
			acceptor_ptr pacc = acceptors_[i];

			pacc->open(ip::tcp::v4(), ec);
			if (ec) return false;

			socket_base::reuse_address opt(true);
			pacc->set_option(opt, ec);
			if (ec) return false;

			ip::tcp::endpoint endpt(ip::tcp::v4(), ports_[i]);
			pacc->bind(endpt, ec);
			if (ec) return false;

			pacc->listen(socket_base::max_connections, ec);
			if (ec) return false;

			accept_some(pacc, 10);
		}
		return true;
	}

	bool	stop()
	{
		stoped_ = true;

		std::vector<remote_socket_ptr> cpl = remotes_;
		for (size_t i = 0; i < cpl.size(); i++)
		{
			cpl[i]->close(false);
		}
		remotes_.clear();

		boost::system::error_code ec;
		for (size_t i = 0; i < acceptors_.size(); i++)
		{
			acceptor_ptr pacc = acceptors_[i];
			pacc->close(ec);
		}
		acceptors_.clear();

		ios_.reset();
		ios_.poll(ec);

		ios_.stop();
		return true;
	}

	void accept_once(acceptor_ptr acc)
	{
		if (!acc.get()) return;
		static int gid = 0;

		remote_socket_ptr remote(new remote_t(*this));
		remote->id_ = gid++;

		auto handle_acc =
			boost::bind(&net_server::on_accept_complete, this,
				boost::asio::placeholders::error,
				acc,
				remote);
		acc->async_accept(remote->s, handle_acc);
	}

	void	accept_some(acceptor_ptr acc, int count)
	{
		for (int i = 0; i < count; i++)
		{
			accept_once(acc);
		}
	}

	std::vector<remote_socket_ptr>		get_remotes()
	{
		std::vector<remote_socket_ptr>	ret;
		{
			ret = remotes_;
		}
		return ret;
	}
	virtual bool	on_connection_accepted(remote_socket_ptr remote)
	{
		return true;
	}
protected:
	void			on_accept_complete(const boost::system::error_code& ec, acceptor_ptr pacc_, remote_socket_ptr remote_)
	{
		if (ec.value() == 0) {
			bool acc = on_connection_accepted(remote_);
			if (acc) {
				remote_->security_check_ = boost::posix_time::microsec_clock::local_time();
				remote_->start_recv();
				remotes_.push_back(remote_);
			}
			accept_once(pacc_);
		}
		else if (!stoped_) {
			accept_once(pacc_);
		}
	}
};
