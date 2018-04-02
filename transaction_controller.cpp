#include "transaction_controller.h"
#include "boost/smart_ptr.hpp"

boost::shared_ptr<transaction_controller> gins_transaction_controller;
transaction_controller* transaction_controller::get_instance()
{
	if (!gins_transaction_controller){
		gins_transaction_controller.reset(new transaction_controller());
	}
	return gins_transaction_controller.get();
}

void transaction_controller::free_instance()
{
	gins_transaction_controller.reset();
}

int transaction_controller::start()
{
	return 0;
}

int transaction_controller::step()
{
	auto it = trans_.begin();
	if (it != trans_.end()){
		trans_ptr tr = it->second;
		int ret = tr->step();
		if (ret == state_failed || ret == state_finished) {
			if (ret == state_failed) {
				if (tr->contex_.return_code_ != 0) {
					if (tr->contex_.send_return_code_){
						tr->contex_.send_return_code_();
					}
				}
			}
			trans_.erase(it);
		}
	}
	return trans_.size();
}

int transaction_controller::stop()
{
	trans_.clear();
	return 0;
}

void transaction_controller::start_transaction(std::string key, trans_ptr tr)
{
	trans_[key] = tr;
	tr->start();
}
