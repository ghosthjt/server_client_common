#pragma once
#include "i_dice_controller.h"
class dice_controller : public i_dice_controller
{
public:
	dice_controller();
	~dice_controller()
	{
		delete current_use_;
	}
	//group 为指定使用某组概率roll点
	void			dice_dice(
		std::vector<dice_result_data>& dice_result,
		int group = -1);

	void			sum_user_bets(std::map<std::string, __int64>& ret);

	__int64			get_total_bets(int bet_id);
	__int64			get_banker_win();
	__int64			get_system_win();

protected:
	std::map<int, dice_item_ptr>*	current_use_;
	std::vector<dice_result_data>	groups_result_;	//已出结果集

	void			get_special_item_groups(std::map<int, dice_item_ptr>& ret);

	bool			is_proper_balance(std::map<std::string, std::map<int, user_get_data>>& user_get, std::string modifer);
		
	void			balance(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get);
	void			balance_group(const std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get);

	//大捕获模式,所有压注项都中奖
	void			balance_all_bet_wins(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get);

	//老虎机拖拉机模式,会roll出连续位置的N个奖项,玩家压中这些算中奖
	void			balance_tractor(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get);
	
	//老虎机流星模式,会随机roll出随机位置的N个奖项,玩家压中这些算中奖
	void			balance_meteor(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get);

	//大乐透返还压过的钱.
	void			feedback_super_lotto(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get);

	void			do_dice_dice(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type);


	//寻找一个对用户来说最好的奖项
	int				find_best_pid_for_user(const std::map<int, __int64>& sumbet);

	//寻找一个对用户来说最坏的奖项
	int				find_worse_pid_for_user(const std::map<int, __int64>& sumbet);

	//正常roll点
	void			dice_normal(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type, int deep = 0);

	//roll出这个玩家赢最大的奖
	void			dice_perfer_user(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type, std::string uid = "all");

	//roll出玩家输最大的奖
	void			dice_perfer_system(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type, std::string uid = "all");
	void			balance_result(
		int pid,
		std::vector<int>& dice_result,
		std::map<std::string, std::map<int, user_get_data>>& user_get,
		int& adjust_type,
		int deep,
		std::string modifer);

	void			dice_in_group(int groupid, std::vector<dice_result_data>& dice_result);
};
