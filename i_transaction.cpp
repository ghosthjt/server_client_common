#include "i_transaction.h"

int i_transaction_item::commit()
{
	return 0;
}

int i_transaction_item::step()
{
	return 0;
}

int i_transaction_item::state()
{
	return state_finished;
}

int i_transaction_item::rollback()
{
	return 0;
}

i_transaction_item::~i_transaction_item()
{

}

transaction_routine::transaction_routine() :counter_("")
{
	static int  gid = 1;
	curpos_ = 0;
	counter_.desc("transaction_routine->" + lx2s(gid++));
}

transaction_routine::~transaction_routine()
{

}

void transaction_routine::push_trans(transi_ptr pi)
{
	trans_.push_back(pi);
}

void transaction_routine::rollback()
{
	for (int i = trans_.size() - 1; i >= 0; i--)
	{
		trans_[i]->rollback();
	}
}


transi_ptr transaction_routine::current()
{
	if (curpos_ < (int)trans_.size()){
		return trans_[curpos_];
	}
	return transi_ptr();
}

int transaction_routine::next()
{
	//已完成
	if (curpos_ >= (int)trans_.size()) {
		return 0;
	}
	else {
		transi_ptr pi = trans_[curpos_];
		curpos_++;
		if (curpos_ >= (int)trans_.size()) {
			return 0;
		}

		int ret = trans_[curpos_]->commit();
		if (ret != 0){
			return 1;
		}
		return 2;
	}
}

int transaction_routine::step()
{
	if (current() == nullptr) return state_finished;
	
	current()->step();

	if (current()->state() == state_success) {
		//如果没有下一步了,从表列中移除

		int ret = next();
		// 如果出错了，回滚
		if (ret == 1){
			rollback();
			return state_failed;
		}
		//如果已经结束
		else if(ret == 0){
			return state_finished;
		}
	}
	else if (current()->state() == state_failed) {
		rollback(); 
		return state_failed;
	}
	else if (current()->state() == state_finished){
		return state_finished;
	}
	return state_ishandling;
}

int transaction_routine::start()
{
	transi_ptr ptrans = current();
	if (ptrans){
		return ptrans->commit();
	}
	return 0;
}
