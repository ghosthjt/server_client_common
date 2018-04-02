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
	//group Ϊָ��ʹ��ĳ�����roll��
	void			dice_dice(
		std::vector<dice_result_data>& dice_result,
		int group = -1);

	void			sum_user_bets(std::map<std::string, __int64>& ret);

	__int64			get_total_bets(int bet_id);
	__int64			get_banker_win();
	__int64			get_system_win();

protected:
	std::map<int, dice_item_ptr>*	current_use_;
	std::vector<dice_result_data>	groups_result_;	//�ѳ������

	void			get_special_item_groups(std::map<int, dice_item_ptr>& ret);

	bool			is_proper_balance(std::map<std::string, std::map<int, user_get_data>>& user_get, std::string modifer);
		
	void			balance(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get);
	void			balance_group(const std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get);

	//�󲶻�ģʽ,����ѹע��н�
	void			balance_all_bet_wins(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get);

	//�ϻ���������ģʽ,��roll������λ�õ�N������,���ѹ����Щ���н�
	void			balance_tractor(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get);
	
	//�ϻ�������ģʽ,�����roll�����λ�õ�N������,���ѹ����Щ���н�
	void			balance_meteor(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get);

	//����͸����ѹ����Ǯ.
	void			feedback_super_lotto(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get);

	void			do_dice_dice(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type);


	//Ѱ��һ�����û���˵��õĽ���
	int				find_best_pid_for_user(const std::map<int, __int64>& sumbet);

	//Ѱ��һ�����û���˵��Ľ���
	int				find_worse_pid_for_user(const std::map<int, __int64>& sumbet);

	//����roll��
	void			dice_normal(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type, int deep = 0);

	//roll��������Ӯ���Ľ�
	void			dice_perfer_user(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type, std::string uid = "all");

	//roll����������Ľ�
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
