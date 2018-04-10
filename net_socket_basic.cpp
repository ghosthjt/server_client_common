#include "net_socket_basic.h"

basic_socket_impl::basic_socket_impl(boost::asio::io_service& ios) :
	ios_(ios),
	s(ios),
	last_socket_error_(0),
	check_data_recv_(ios)
{
	close_when_idle_ = false;
	//作为服务器端接受的连接使用时,默认状态应该是已连接状态.
	work_state = work_state_closed;
	recv_helper_.reading_buffer_ = stream_buffer(boost::shared_array<char>(new char[32768]), 0, 32768);
	recv_helper_.is_recving_ = false;
	send_helper_.is_sending_ = false;
	create_in_thread_ = boost::this_thread::get_id();
}

void basic_socket_impl::close(bool passive /*= false*/)
{
	boost::system::error_code ec;
	check_data_recv_.cancel(ec);
	work_state = work_state_closed;
	close_when_idle_ = false;
	s.shutdown(socket_base::shutdown_both, ec);
	s.close(ec);

	//未知的套接字错误
	if (last_socket_error_ == 0)
		last_socket_error_ = -1;
}

void basic_socket_impl::add_to_send_queue(stream_buffer dat, bool close_this /*= false*/)
{
	if (!isworking()) return;

	//不是创建者线程不可以发送消息
	if (boost::this_thread::get_id() != create_in_thread_) {

		std::cout << "warning!!!-->async sending data in other thread. head->[" << "]" << std::endl;

		ios_.post(boost::bind(&basic_socket_impl::switch_to_create_thread,
			this->shared_from_this(), dat, close_this, false));

		return;
	}
	else {
		send_helper_.send_vct.push_back(dat);

		if (!send_helper_.is_sending_) {
			send_helper_.is_sending_ = true;
			if (!send_new_buffer()) {
				send_helper_.is_sending_ = false;
			}
		}

		if (close_this) {
			close_when_idle_ = true;
		}
	}
}

int basic_socket_impl::send_data(stream_buffer dat)
{
	if (boost::this_thread::get_id() != create_in_thread_) {

		std::string ss(dat.data(), std::min<int>(20, dat.data_left()));
		std::cout << "warning!!!-->sending data in other thread. head->["
			<< ss << "]" << std::endl;

		ios_.post(boost::bind(&basic_socket_impl::switch_to_create_thread,
			this->shared_from_this(), dat, false, true));
		return 0;
	}

	boost::shared_ptr<stream_buffer> pdat(new stream_buffer());
	*pdat = dat;
	boost::shared_ptr<boost::system::error_code> pec(new boost::system::error_code());

	//使用异步来模拟同步发送
	s.async_send(boost::asio::buffer(pdat->data(), pdat->data_left()),
		boost::bind(&basic_socket_impl::on_data_block_sended,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred, pdat, pec));

	time_counter ts;
	ts.restart();
	do{
		ios_.poll_one();
	} 
	while (pdat->data_left() > 0 && pec->value() == 0 && ts.elapse() < 300);

	if (pdat->data_left() == 0) {
		return 0;
	}
	else {
		return -1;
	}
}

int basic_socket_impl::send_data(std::string dat)
{
	boost::shared_array<char> sh(new char[dat.size()]);
	memcpy(sh.get(), dat.c_str(), dat.size());
	stream_buffer buf(sh, dat.size(), dat.size());
	return send_data(buf);
}

boost::asio::ip::tcp::endpoint basic_socket_impl::remote_endpoint()
{
	boost::system::error_code ec;
	return s.remote_endpoint(ec);
}

int basic_socket_impl::pickup_data(void* out, unsigned int len, bool remove /*= false*/, bool all /*= false*/)
{
	int rl = 0;
	if (all) {
		rl = std::min<unsigned int>(len, recv_helper_.reading_buffer_.data_left());
	}
	else {
		if (recv_helper_.reading_buffer_.data_left() < len) return 0;
		rl = len;
	}

	memcpy(out, recv_helper_.reading_buffer_.data(), rl);
	if (remove) {
		recv_helper_.reading_buffer_.use_data(rl);
	}

	return rl;
}

void basic_socket_impl::start_recv()
{
	work_state = work_state_working;
	recv_helper_.is_recving_ = true;
	s.async_receive(
		boost::asio::buffer(recv_helper_.reading_buffer_.buffer(), recv_helper_.reading_buffer_.buffer_left()),
		boost::bind(&basic_socket_impl::data_recv_handler,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			false));
}

void basic_socket_impl::switch_to_create_thread(stream_buffer dat, bool close_this, bool sync)
{
	if (sync) {
		send_data(dat);
	}
	else {
		add_to_send_queue(dat, close_this);
	}
}

void basic_socket_impl::on_recv_check(const boost::system::error_code& ec)
{
	if (ec.value() == 0) {
		data_recv_handler(boost::system::error_code(), 0, true);
	}
}

void basic_socket_impl::data_recv_handler(const boost::system::error_code& ec, size_t bytes_transferred, bool ischeck)
{
	if (!isworking()) return;
	if (ec.value() == 0) {
		recv_helper_.reading_buffer_.use_buffer(bytes_transferred);
		int ret = on_data_recv();
		if (ret == 0) {
			if (recv_helper_.reading_buffer_.buffer_left() < 128) {
				recv_helper_.reading_buffer_.remove_used();
			}

			//如果不是超时回调,则重置接收状态
			if (!ischeck) {
				recv_helper_.is_recving_ = false;
			}

			//如果缓冲有空闲,并且没有在接收状态
			if (recv_helper_.reading_buffer_.buffer_left() > 0 && !recv_helper_.is_recving_) {
				recv_helper_.is_recving_ = true;
				s.async_receive(
					boost::asio::buffer(recv_helper_.reading_buffer_.buffer(), recv_helper_.reading_buffer_.buffer_left()),
					boost::bind(&basic_socket_impl::data_recv_handler,
						this->shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred,
						false));
			}
			else if (recv_helper_.reading_buffer_.buffer_left() == 0) {
				if (option("close_on_rdbuffer_full") == "1") {
					last_socket_error_ = 10055;
					close(true);
				}
				else {
					recv_helper_.is_recving_ = false;
					//缓冲区满了,10ms后再读数据，让目前的数据有时间处理
					check_data_recv_.expires_from_now(boost::posix_time::millisec(1));
					check_data_recv_.async_wait(
						boost::bind(&basic_socket_impl::on_recv_check,
							this->shared_from_this(),
							boost::asio::placeholders::error));
				}
			}
		}
	}
	else {
		if (work_state != work_state_closed) {
			last_socket_error_ = ec.value();
			close(true);
		}
	}
}

bool basic_socket_impl::send_new_buffer()
{
	if (!send_helper_.send_vct.empty()) {
		send_helper_.sending_buffer_ = send_helper_.send_vct.front();
		send_helper_.send_vct.pop_front();
	}
	if (!send_helper_.sending_buffer_.is_complete()) {
		s.async_send(boost::asio::buffer(send_helper_.sending_buffer_.data(), send_helper_.sending_buffer_.data_left()),
			boost::bind(&basic_socket_impl::on_data_sended_notify,
				this->shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		return true;
	}
	return false;
}

void basic_socket_impl::on_data_sended_notify(const boost::system::error_code& ec, size_t byte_transfered)
{
	if (ec.value() != 0) {
		send_helper_.is_sending_ = false;
	}
	else {
		send_helper_.sending_buffer_.use_data(byte_transfered);
		if (send_helper_.sending_buffer_.is_complete()) {
			if (!send_new_buffer()) {
				if (close_when_idle_) {
					ios_.post(boost::bind(&basic_socket_impl::close,
						this->shared_from_this(),
						false));
				}
				send_helper_.is_sending_ = false;
			}
		}
		//没发完，继续发剩下的
		else {
			s.async_send(boost::asio::buffer(send_helper_.sending_buffer_.data(), send_helper_.sending_buffer_.data_left()),
				boost::bind(&basic_socket_impl::on_data_sended_notify,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}
}

void basic_socket_impl::on_data_block_sended(
	const boost::system::error_code& ec, size_t byte_transfered,
	boost::shared_ptr<stream_buffer> pdat,
	boost::shared_ptr<boost::system::error_code> pec)
{
	if (ec.value() == 0) {
		pdat->use_data(byte_transfered);
		if (pdat->data_left() == 0) return; 

		s.async_send(boost::asio::buffer(pdat->data(),
			pdat->data_left()),
			boost::bind(&basic_socket_impl::on_data_block_sended,
				this->shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred, pdat, pec));
	}
	else {
		*pec = ec;
	}
}

