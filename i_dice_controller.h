#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "boost/shared_ptr.hpp"
#include "balance_controller.h"

#include "utility.h"
enum
{
	//无效的奖项
	special_item_invalid = -1,	
	special_item_normal,
	//大捕获模式,所有压注项都中奖(区分group_id)  --海底大冒险，丛林争霸通赔模式
	special_item_all_bet_wins = -100,

	//大乐秀模式,中奖之后不光有倍数,还返还所有压注金额,
	special_item_super_lotto,

	//老虎机拖拉机模式,会roll出连续位置的N个奖项,玩家压中这些算中奖
	special_item_tractor,

	//老虎机流星模式,会随机roll出随机位置的N个奖项,玩家压中这些算中奖
	special_item_meteor,

	special_item_group,
};

//奖项设置
struct dice_item
{
	int		pid_;			//奖项id
	int		func_id_;		//
	int		rate_;			//概率
	float	factor_;		//倍率
	int		bet_id_;		//压注项id
	int		max_count_;		//每个跨度开奖最多出几次
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

//压注项设置
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

//结算守护者
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

	//回馈用户
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

	//从配置文件中读取的数据
	int		func_type_;
	std::map<int, dice_item_ptr> dices_1;
	int		auto_balance_user_;
	float	wincap_, losecap_, feedback_factor_, max_sysbal_;//最多赢取库存百分比
	std::map<int, float> group_bet_factors_;//每个组合的投注权值.

	//运行时设置的数据
	__int64 banker_deposit_;//庄家当前资金
	int		user_mode_;		//1家庄是真实玩家 0系统庄

	std::string banker_uid_;
	//从组别号开始,组别号结束
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
