#include "transactions.h"
#include "i_log_system.h"
#include "db_delay_helper.h"

transi_callfunc::transi_callfunc()
{
	ec_ = -1;
}

void transi_callfunc::setcb(std::function<int()> func_commit, std::function<int()> func_rollback /*= nullptr*/)
{
	func_commit_ = func_commit;
	func_rollback_ = func_rollback;
}

int transi_callfunc::commit()
{
	ec_ = func_commit_();
	return ec_;
}

int transi_callfunc::state()
{
	return ec_;
}

int transi_callfunc::rollback()
{
	if (func_rollback_) {
		func_rollback_();
	}
	return 0;
}

transi_dbquery::transi_dbquery(std::function<std::string()> sql_builder,
	db_delay_helper& db):db_(db)
{
	sql_builder_ = sql_builder;
	state_ = state_ishandling;
}

int transi_dbquery::commit()
{
	boost::weak_ptr<transi_dbquery> wqr = boost::dynamic_pointer_cast<transi_dbquery>(shared_from_this());
	std::string sql = sql_builder_();
	db_.get_result(sql, [sql, wqr](bool result, const std::vector<result_set_ptr> &v) {
		boost::shared_ptr<transi_dbquery> qr = wqr.lock();
		if (qr){
			qr->on_dbresult(result, v);
		}
		else {
			i_log_system::get_instance()->write_log(loglv_warning, "query [%s] is finished, but the transaction was not valid.", sql.c_str());
		}
	});
	return 0;
}

int transi_dbquery::state()
{
	return state_;
}

int transi_dbquery::rollback()
{
	return 0;
}


void transi_dbquery::on_dbresult(bool result, const std::vector<result_set_ptr> &v)
{
	if (result){
		if (v.empty()){
			i_log_system::get_instance()->write_log(loglv_warning, "mysql query success, but no recordset found.");
			state_ = state_failed;
		}
		else {
			state_ = state_success;
			auto ptrans = wtrans_.lock();
			if (ptrans) {
				ptrans->contex_.result_ = v;
			}
		}
	}
	else {
		state_ = state_failed;
	}
}

int transi_dbquery_handler::commit()
{
	if (!func_commit_){
		i_log_system::get_instance()->write_log(loglv_warning, "transi_dbquery_handler::commit() called without func_commit_.");
		return 0;
	}

	ec_ = func_commit_();
	return 0;
}
