//远程事务
#pragma once
#include "boost/smart_ptr.hpp"
#include <vector>
#include <map>
#include "function_time_counter.h"

enum
{
	state_ishandling,
	state_success,
	state_failed,
	state_finished,
	state_uncertain,
};

struct sql_query_result_set;
typedef boost::shared_ptr<sql_query_result_set>	result_set_ptr;

struct	transaction_routine_contex
{
	int		return_code_;
	std::vector<result_set_ptr> result_;
	std::map<std::string, std::string> data_;
	std::function<void()>	send_return_code_;
	transaction_routine_contex()
	{
		return_code_ = 0;
	}
};

class	transaction_routine;
typedef boost::shared_ptr<transaction_routine> trans_ptr;
typedef boost::weak_ptr<transaction_routine> trans_wptr;
//事务单元
class i_transaction_item;
typedef boost::shared_ptr<i_transaction_item> transi_ptr;
class i_transaction_item : public boost::enable_shared_from_this<i_transaction_item>
{
public:
	//事件执行
	virtual int		commit();
	virtual int		step();
	virtual int		state();
	virtual int		rollback();

	virtual ~i_transaction_item();
	trans_wptr		wtrans_;
};

//事务
class	transaction_routine
{
public:

	transaction_routine_contex contex_;

	transaction_routine();
	~transaction_routine();
	void	push_trans(transi_ptr pi);
	int		step();
	int		start();

protected:
	std::vector<transi_ptr> trans_;
	int		curpos_;
	function_time_counter	counter_;
	transi_ptr	current();
	int		next();
	void	rollback();
};

