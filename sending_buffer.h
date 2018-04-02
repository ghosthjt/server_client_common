#pragma once
#include <boost/smart_ptr.hpp>
struct stream_buffer
{
	stream_buffer()
	{
		buffer_len_ = 0;
		clear();
	}

	stream_buffer(boost::shared_array<char> dat, unsigned int dlen, unsigned int blen)
	{
		used_ = 0;
		data_len_ = dlen;
		data_ = dat;
		buffer_len_ = blen;
	}
	void	clear()
	{
		used_ = 0;
		data_len_ = 0;
	}

	inline char*			data()
	{
		if (!data_.get()) {
			return NULL;
		}
		return data_.get() + used_;
	}

	inline char*			buffer()
	{
		if (!data_.get()) {
			return NULL;
		}
		return data_.get() + data_len_;
	}

	inline unsigned int	buffer_left()
	{
		return buffer_len_ - data_len_;
	}

	inline unsigned int	data_left()
	{
		return data_len_ - used_;
	}

	inline bool			is_complete()
	{
		return	data_left() == 0;
	}

	inline void			use_data(unsigned int len)
	{
		used_ += len;
	}

	inline void			use_buffer(unsigned int len)
	{
		data_len_ += len;
	}

	inline void			remove_used()
	{
		if (used_ > 0) {
			memmove(data_.get(), data_.get() + used_, data_left());
			data_len_ = data_left();
			used_ = 0;
		}
	}

	inline unsigned int buffer_len()
	{
		return buffer_len_;
	}

private:
	unsigned int		used_;
	boost::shared_array<char>	data_;
	unsigned int		data_len_;
	unsigned int		buffer_len_;

};