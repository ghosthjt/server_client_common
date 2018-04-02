#pragma once
#include "i_transaction.h"
#include "boost/smart_ptr.hpp"

struct sql_query_result_set;
typedef boost::shared_ptr<sql_query_result_set>	result_set_ptr;

//回调函数
class transi_callfunc : public i_transaction_item
{
public:
	transi_callfunc();

	void setcb(std::function<int()> func_commit,
		std::function<int()> func_rollback = nullptr);
	virtual int commit() override;
	virtual int state() override;
	virtual int rollback() override;

protected:
	int			ec_;
	std::function<int()> func_commit_, func_rollback_;
};

//数据库查询回调
class transi_dbquery_handler : public transi_callfunc
{
public:
	virtual int commit() override;
};


//数据库查询
//保证有数据才会继续下去,否则流程失败,回滚.
struct db_delay_helper;
class transi_dbquery : public i_transaction_item
{
public:
	transi_dbquery(std::function<std::string()> sql_builder, db_delay_helper& db);

	virtual int commit() override;
	virtual int state() override;
	virtual int rollback() override;

	void	on_dbresult(bool result, const std::vector<result_set_ptr> &v);
protected:
	std::function<std::string()> sql_builder_;
	int			state_;
	db_delay_helper& db_;
};


#define BEGIN_TRANS(cls)\
	trans_ptr ptrans(new transaction_routine());\
	boost::shared_ptr<cls> pthis = boost::dynamic_pointer_cast<cls>(shared_from_this());\
	int placeholder = 0;

#define END_TRANS(skey)\
	transaction_controller::get_instance()->start_transaction(skey, ptrans);

#define ADD_QUERY_TRANS(sql, db)\
{\
	transi_ptr pi(new transi_dbquery(sql, db));\
	pi->wtrans_ = ptrans;\
	ptrans->push_trans(pi);\
}

#define BEGIN_QUERY_CALLBACK(ec, ...)\
{\
	boost::shared_ptr<transi_dbquery_handler> pi(new transi_dbquery_handler());\
	auto fn = [ptrans, pthis, pi, ##__VA_ARGS__]()->int {\
		if (ptrans->contex_.result_.empty()) {\
			ptrans->contex_.return_code_ = ec;\
			return state_failed;			\
		}									

#define READ_ONE_ROW(ec)\
		query_result_reader q(ptrans->contex_.result_[0]);\
		if(!q.fetch_row()){\
			ptrans->contex_.return_code_ = ec; \
			return state_failed;			\
		}
		
#define READ_ALL_ROW()\
		query_result_reader q(ptrans->contex_.result_[0]);\
		while(q.fetch_row()){\

#define END_READ_ALL_ROW()\
		}

#define END_CALLBACK()\
	return state_success;\
	};\
	pi->setcb(fn, nullptr);\
	pi->wtrans_ = ptrans;\
	ptrans->push_trans(pi);\
}

#define CHECK_EXISTS(ec)\
{\
	boost::shared_ptr<transi_dbquery_handler> pi(new transi_dbquery_handler()); \
	auto fn = [ptrans, pthis, pi]()->int {\
		if (ptrans->contex_.result_.empty()) {\
			return state_success;\
		}\
		query_result_reader q(ptrans->contex_.result_[0]);\
		if(q.fetch_row()){\
			ptrans->contex_.return_code_ = ec; \
			return state_failed;\
		}\
		return state_success;\
	};\
	pi->setcb(fn, nullptr);\
	pi->wtrans_ = ptrans;\
	ptrans->push_trans(pi);\
}

#define BEGIN_FUNCTION(...)\
{\
	boost::shared_ptr<transi_callfunc> pi(new transi_callfunc());\
	pi->wtrans_ = ptrans;\
	auto fn = [ptrans, pthis, __VA_ARGS__]()->int {
