#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "boost/shared_ptr.hpp"
#include "balance_controller.h"

#include "utility.h"
enum
{
	//��Ч�Ľ���
	special_item_invalid = -1,	
	special_item_normal,
	//�󲶻�ģʽ,����ѹע��н�(����group_id)  --���״�ð�գ���������ͨ��ģʽ
	special_item_all_bet_wins = -100,

	//������ģʽ,�н�֮�󲻹��б���,����������ѹע���,
	special_item_super_lotto,

	//�ϻ���������ģʽ,��roll������λ�õ�N������,���ѹ����Щ���н�
	special_item_tractor,

	//�ϻ�������ģʽ,�����roll�����λ�õ�N������,���ѹ����Щ���н�
	special_item_meteor,

	special_item_group,
};

//��������
struct dice_item
{
	int		pid_;			//����id
	int		func_id_;		//
	int		rate_;			//����
	float	factor_;		//����
	int		bet_id_;		//ѹע��id
	int		max_count_;		//ÿ����ȿ�����������
	int		group_id_;
	std::vector<int> win_for_;
	std::vector<int> exclude_;
	std::string name_;
	bool	operator == (dice_item& pr)
	{
		return (pr.pid_ == pid_) && (pr.rate_ == rate_) && (pr.factor_ == factor_) && (pr.bet_id_ == bet_id_);
	}
	dice_item();
};

typedef boost::shared_ptr<dice_item> dice_item_ptr;

//ѹע������
struct bet_item
{
	int			bet_id_;
	__int64		cost_;
	bet_item()
	{
		bet_id_ = 0;
		cost_ = 0;
	}
};

class dice_controller;

struct	user_get_data
{
	int			bet_id_;
	int			pid_;
	__int64		win_;
	int			pid2_;
	user_get_data()
	{

		bet_id_ = pid2_ = pid_ = 0;
		win_ = 0;
	}
};


struct dice_result_data 
{
	std::vector<int> dice_result_;
	int		adjust_type_;
	int		group_;
	std::map<std::string, std::map<int, user_get_data>> user_get_;
	dice_result_data();
	void	clear();
};

class i_dice_controller;

//�����ػ���
class	balance_warden : public balance_controller
{
public:
	balance_warden();

	void	update_warden(
		const std::map<std::string, std::vector<bet_item>>& bets,
		const std::vector<dice_result_data>& dice_result,
		const std::map<std::string, int>& user_data,
		const i_dice_controller* dc);

	void	next_turn(const std::vector<dice_result_data>& dice_result);

	//�����û�
	int		feedback_user(
		std::string& uid,
		std::map<std::string, std::vector<bet_item>>& bets,
		const std::map<int, dice_item_ptr>& dice_items,
		i_dice_controller* dc);

	void	user_pay(std::string uid, __int64 count);
	void	clear_user_pay(std::string uid);

	void	heart_beat();

protected:
	std::map<int, int>		warden_dice_distribution_;
	int						turn_;
	bool					inited_;
	time_t					last_beat_;
};

struct dice_controller_config
{
	enum 
	{
		func_type_normal,
		func_type_slot,
	};

	//�������ļ��ж�ȡ������
	int		func_type_;
	std::map<int, dice_item_ptr> dices_1;
	int		auto_balance_user_;
	float	wincap_, losecap_, feedback_factor_, max_sysbal_;//���Ӯȡ���ٷֱ�
	std::map<int, float> group_bet_factors_;//ÿ����ϵ�ͶעȨֵ.

	//����ʱ���õ�����
	__int64 banker_deposit_;//ׯ�ҵ�ǰ�ʽ�
	int		user_mode_;		//1��ׯ����ʵ��� 0ϵͳׯ

	std::string banker_uid_;
	//�����ſ�ʼ,���Ž���
	dice_controller_config();

	bool	read_config(std::string file_name);
	bool	save_config(std::string file_name);
	bool	read_bet_group(std::string strgp);

	std::string get_bet_group();

	void	reset();
};


class i_dice_controller
{
public:
	dice_controller_config conf_;
	std::map<std::string, std::vector<bet_item>> bets_;
	std::map<std::string, int>	user_data_;
	balance_warden*	warden_;
	__int64			old_banker_deposit_;
	int				dice_point_;
	virtual ~i_dice_controller(){}
	bool	config(dice_controller_config& conf, balance_warden* war);
	bool	set_bet(std::string uid, int is_bot, int bet_id, __int64 cnt, int groupid, bool istest = false, __int64 cap = 0xffffffff);
	bool	clear_bets(std::string uid);
	int		do_dice(const std::map<int, dice_item_ptr>& dices, bool sumrate = false);
	void	reset_user_data();
	virtual void	dice_dice(std::vector<dice_result_data>& dice_result,int group = -1) = 0;
};
