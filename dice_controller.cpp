#include "dice_controller.h"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "json_helper.h"
#include <algorithm>

extern void		sum_bet(
	const i_dice_controller* dc,
	const std::map<std::string, std::vector<bet_item>>& bets,
	std::string uid,
	std::map<int, __int64>& ret, 
	bool detail);

//根据下注项得到对应的奖项列表
void	get_dice_by_betid(const std::map<int, dice_item_ptr>& dice, int bet_id, std::vector<dice_item_ptr>& ret)
{
	auto it = dice.begin();
	while (it != dice.end())
	{
		if (it->second->bet_id_ == bet_id) {
			ret.push_back(it->second);
		}
		it++;
	}

	std::sort(ret.begin(), ret.end(), [](const dice_item_ptr p1, const dice_item_ptr p2)->bool {
		return p1->factor_ > p2->factor_;
	});
}

static void		evalute_pay(const std::map<int, dice_item_ptr>& dice,
							const std::map<int, __int64>& sumbet,
							std::map<int, __int64>& each_pid_willpay)
{
	auto itw = std::find_if(dice.begin(), dice.end(),
		[](const std::pair<int, dice_item_ptr>& it)->bool{
			return it.second->func_id_ == special_item_all_bet_wins;
	});

	auto it = sumbet.begin();
	while (it != sumbet.end())
	{
		std::vector<dice_item_ptr> vitem;
		get_dice_by_betid(dice, it->first, vitem);
		for (unsigned i = 0; i < vitem.size(); i++)
		{
			dice_item_ptr di = vitem[i];
			//不出特殊奖
			if (di->func_id_ == special_item_normal || di->func_id_ == special_item_super_lotto){
				each_pid_willpay[di->pid_] += di->factor_ * it->second;
			}
		}
		it++;
	}
}

balance_warden::balance_warden()
{
	turn_ = 0;
	inited_ = false;
	last_beat_ = 0;
}

void balance_warden::update_warden(
	const std::map<std::string, std::vector<bet_item>>& bets,
	const std::vector<dice_result_data>& dice_result,
	const std::map<std::string, int>& user_data,
	const i_dice_controller* dc)
{
	//先扣除玩家压注额
	{
		auto it = bets.begin();
		while (it != bets.end())
		{
			std::map<int, __int64> sum_bets;
			sum_bet(dc, bets, it->first, sum_bets, false);
			if (!sum_bets.empty()) {
				//玩家总的下了多少钱
				__int64 total_bet = std::accumulate(sum_bets.begin(), sum_bets.end(), 0,
					[](__int64 val, const std::pair<int, __int64>& lmbit)->__int64 {
					return val + lmbit.second;
				});

				auto itf = user_data.find(it->first);
				//玩家下注才记录库存,机器人下注不计入
				if (itf != user_data.end() && itf->second == 0){
					add_user_balance(it->first, -total_bet);
				}
			}
			it++;
		}
	}

	//再加上玩家赢的钱
	for (unsigned i = 0; i < dice_result.size(); i++)
	{
		const dice_result_data& di = dice_result[i];
		auto it = di.user_get_.begin();
		while (it != di.user_get_.end())
		{
			auto itf = user_data.find(it->first);
			//玩家赢钱才记录,机器人不记录
			if (itf != user_data.end() && itf->second == 0) {
				const std::map<int, user_get_data>& wins = it->second;
				auto itv = wins.begin();
				while (itv != wins.end())
				{
					__int64 w = itv->second.win_;
					if (di.adjust_type_ == auto_balance_prefer_user){
						w = dc->conf_.feedback_factor_ * w;
					}
					add_user_balance(it->first, w);
					itv++;
				}
			}
			it++;
		}
	}
}

void balance_warden::next_turn(const std::vector<dice_result_data>& dice_result)
{
	for (unsigned i = 0; i < dice_result.size(); i++)
	{
		for (unsigned ii = 0; ii < dice_result[i].dice_result_.size(); ii++)
		{
			warden_dice_distribution_[dice_result[i].dice_result_[ii]]++;
		}
	}
	turn_++;
}

int balance_warden::feedback_user(std::string& uid, 
								  std::map<std::string, std::vector<bet_item>>& bets,
								  const std::map<int, dice_item_ptr>& dice_items,
								  i_dice_controller* dc)
{

	std::vector<std::pair<std::string, warden_user_data>> vub;
	std::accumulate(warden_user_balance_.begin(), warden_user_balance_.end(), 0,
		[&bets, &vub](int v, const std::pair<std::string, warden_user_data> &p)->int {
		if (bets.find(p.first) != bets.end()){
			vub.push_back(p);
		}
		return 0;
	});

	if (vub.empty())
		return auto_balance_normal;
//////////////////////////////////////////////////////////////////////////
//看是否需要坑玩家
//////////////////////////////////////////////////////////////////////////
	auto itmax = std::max_element(vub.begin(), vub.end(),
		[](const std::pair<std::string, warden_user_data>& pmax,
			const std::pair<std::string, warden_user_data>& p2)->bool {
		return pmax.second.balance_ < p2.second.balance_;
	});

	if (itmax != vub.end()){
		//先看有没有玩家赢了很多钱,如果有,则坑之.
		if (itmax->second.balance_ > itmax->second.avg_bet() * dc->conf_.wincap_ && itmax->second.avg_bet() > 0) {
			double dis = itmax->second.balance_ -  itmax->second.avg_bet() * dc->conf_.wincap_;
			//随机一把看是不是要坑他
			if (rand_r(10000) < 3000)	{
				uid = itmax->first;
				return auto_balance_prefer_system;
			}
			else {
				return auto_balance_normal;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////
//看是否需要救玩家
//////////////////////////////////////////////////////////////////////////
	auto itmin = std::min_element(vub.begin(), vub.end(),
		[](const std::pair<std::string, warden_user_data>& p1,
			const std::pair<std::string, warden_user_data>& pmin)->bool {
		int avg1 = p1.second.avg_bet();
		int avg2 = pmin.second.avg_bet();
		if (avg1 == 0) {
			avg1 = 1;
		}
		if (avg2 == 0) {
			avg2 = 1;
		}
		return (pmin.second.balance_ / avg2) > (p1.second.balance_ / avg1);
	});

	if (itmin == vub.end()){
		return auto_balance_normal;
	}

	uid = itmin->first;

	std::map<int, __int64> smb;
	sum_bet(dc, bets, uid, smb, true);
	if (smb.empty()) {
		return auto_balance_normal;
	}

	std::map<int, __int64> each_pid_willpay;
	evalute_pay(dice_items, smb, each_pid_willpay);
	
	auto itmax3 = std::max_element(each_pid_willpay.begin(), each_pid_willpay.end(),
		[](const std::pair<int, __int64>& p1, const std::pair<int, __int64>& p2)->bool {
		return p1.second < p2.second;
	});
	
	if (itmax3 != each_pid_willpay.end()){
		__int64 lose1 = dc->conf_.losecap_ * itmin->second.avg_bet() * dc->conf_.wincap_;
		__int64 lose2 = itmax3->second * dc->conf_.losecap_;

		if ((itmin->second.balance_ + lose1 < 0)  && (itmin->second.balance_ + lose2 < 0)) {
			if (rand_r(10000) < 7000) {
				return auto_balance_prefer_user;
			}
		}
	}
	return auto_balance_normal;
}

void balance_warden::user_pay(std::string uid, __int64 count)
{
	warden_user_balance_[uid].pay_count_++;
	warden_user_balance_[uid].total_pay_ += count;
}

void balance_warden::clear_user_pay(std::string uid)
{
	warden_user_balance_[uid].pay_count_ = 0;
	warden_user_balance_[uid].total_pay_ = 0;
}

void balance_warden::heart_beat()
{
	if (last_beat_ == 0){
		last_beat_ = time(nullptr);
	}
	else{
		int dis = time(nullptr) - last_beat_;
		//1分钟衰减一次库存
		if (dis > 60){
			last_beat_ = time(nullptr);
			decay_sysbalance();
		}
	}
}

dice_controller::dice_controller()
{
	warden_ = nullptr;
	current_use_ = nullptr;
	dice_point_ = -1;
}


void dice_controller::dice_dice(std::vector<dice_result_data>& dice_result,
								int group)
{
	if (conf_.dices_1.empty())	return;

	if (!current_use_){
		current_use_ = new std::map<int, dice_item_ptr>();
	}

	//所有组合roll
	if (group == -1){
		std::set<int> groups;
		std::for_each(conf_.dices_1.begin(), conf_.dices_1.end(),
			[&groups](const std::pair<int, dice_item_ptr>& lmbit) {
			groups.insert(lmbit.second->group_id_);
		});

		std::for_each(groups.begin(), groups.end(),
			[this, &dice_result](int groupid) {
			dice_in_group(groupid, dice_result);
		});
	}
	//roll特定组
	else {
		dice_in_group(group, dice_result);
	}

	warden_->update_warden(bets_, dice_result, user_data_, this);
	
	//如果用户做庄模式,则需要把用户庄赢的钱计入系统库存,因为玩家赢钱=系统输钱.
	//因为有些游戏庄家不用下注,在update_warden中只记录了闲家的输赢,
	//所以这里要补上庄家的输赢.
	if (conf_.user_mode_){
		warden_->add_user_balance(conf_.banker_uid_, get_banker_win());
	}
	warden_->next_turn(dice_result);
}

void dice_controller::sum_user_bets(std::map<std::string, __int64>& ret)
{
	auto it = bets_.begin();
	while (it != bets_.end())
	{
		std::map<int, __int64> v;
		sum_bet(nullptr, bets_,  it->first, v, false);
		if (!v.empty())	{
			ret.insert(std::make_pair(it->first, v.begin()->second));
		}
		it++;
	}
}

__int64 dice_controller::get_total_bets(int bet_id)
{
	std::map<int, __int64> v;
	sum_bet(nullptr, bets_, "all", v, true);
	return v[bet_id];
}

__int64 dice_controller::get_banker_win()
{
	return old_banker_deposit_ - conf_.banker_deposit_;
}

__int64 dice_controller::get_system_win()
{
	if (!conf_.user_mode_){
		return get_banker_win();
	}
	else {
		return 0;
	}
}


void dice_controller::dice_normal(std::vector<int>& dice_result, 
								  std::map<std::string, std::map<int, user_get_data>>& user_get,
								  int& adjust_type, 
								  int deep /*= 0*/)
{
	int pid = -1;
	while (1)
	{
		pid = do_dice(*current_use_);
		if (current_use_->find(pid) != current_use_->end()) {
			bool is_exclude = false;
			auto & di = (*current_use_)[pid];
			for (int i = 0; i < groups_result_.size(); ++i)
			{
				for (int j = 0; j < groups_result_[i].dice_result_.size(); ++j)
				{
					if (std::find(di->exclude_.begin(), di->exclude_.end(), groups_result_[i].dice_result_[j]) != di->exclude_.end()) {
						is_exclude = true;
					}
				}
			}

			if (!is_exclude) {
				break;
			}
		}
	}
	
	balance_result(pid, dice_result, user_get, adjust_type, deep, "");
}

void dice_controller::dice_perfer_user(std::vector<int>& dice_result,
									   std::map<std::string, std::map<int, user_get_data>>& user_get, 
									   int& adjust_type, 
									   std::string uid /*= "all"*/)
{
	std::map<int, __int64> sumb;
	sum_bet(this, bets_, uid, sumb, true);
	//如果要救的人没压注,则正常出.
	if (sumb.empty()) {
		dice_normal(dice_result, user_get, adjust_type);
	}
	else {
		int pid = find_best_pid_for_user(sumb);
		if (pid == special_item_invalid){
			dice_normal(dice_result, user_get, adjust_type);
		}
		else{
			adjust_type = balance_warden::auto_balance_prefer_user;
			balance_result(pid, dice_result, user_get, adjust_type, 0, uid);
		}
	}
}

void dice_controller::dice_perfer_system(std::vector<int>& dice_result,
										 std::map<std::string, std::map<int, user_get_data>>& user_get,
										 int& adjust_type, 
										 std::string uid /*= "all"*/)
{
	std::map<int, __int64> sumb;
	sum_bet(this, bets_, uid, sumb, true);
	//如果要坑的人没压注,则正常出
	if (sumb.empty()) {
		dice_normal(dice_result, user_get, adjust_type);
	}
	else {
		int pid = find_worse_pid_for_user(sumb);
		if (pid == special_item_invalid){
			dice_normal(dice_result, user_get, adjust_type);
		}
		else{
			adjust_type = balance_warden::auto_balance_prefer_system;
			balance_result(pid, dice_result, user_get, adjust_type, 0, "");
		}
	}
}

void dice_controller::balance_result(
	int pid,
	std::vector<int>& dice_result,
	std::map<std::string, std::map<int, user_get_data>>& user_get,
	int& adjust_type, 
	int deep,
	std::string modifer)
{
	dice_result.push_back(pid);

	if (pid == special_item_invalid) {
		return;
	}

	dice_item_ptr& di = (*current_use_)[pid];
	if (di->func_id_ == special_item_all_bet_wins) {
		balance_all_bet_wins(di->pid_, user_get);
	}
	else if (di->func_id_ == special_item_tractor) {
		balance_tractor(dice_result, user_get);
	}
	else if (di->func_id_ == special_item_meteor) {
		balance_meteor(dice_result, user_get);
	}
	else {
		balance(pid, user_get);
		//返回玩家压注钱
		if (di->func_id_ == special_item_super_lotto) {
			feedback_super_lotto(pid, user_get);
		}
	}

	if (!is_proper_balance(user_get, modifer) && deep < 10 && conf_.auto_balance_user_) {

		adjust_type = balance_warden::auto_balance_redistribute_by_ctrl;
		dice_result.clear();
		user_get.clear();
		dice_normal(dice_result, user_get, adjust_type, ++deep);
		return;
	}

}

void dice_controller::dice_in_group(int groupid, std::vector<dice_result_data>& dice_result)
{
	current_use_->clear();
	std::for_each(conf_.dices_1.begin(), conf_.dices_1.end(),
		[this, groupid](const std::pair<int, dice_item_ptr>& it) {
		if ((*it.second).group_id_ == groupid) {
			(*current_use_)[it.first] = it.second;
		}
	});

	if (!current_use_->empty()) {
		dice_result_data dat;
		dat.group_ = groupid;
		do_dice_dice(dat.dice_result_, dat.user_get_, dat.adjust_type_);
		balance_group(dat.dice_result_, dat.user_get_);
		dice_result.push_back(dat);
		groups_result_.push_back(dat);
		
		std::string str_sets;
		std::for_each(current_use_->begin(), current_use_->end(), 
			[&str_sets](const std::pair<int, dice_item_ptr>& it) {
			str_sets += "[pid:" + lx2s(it.second->pid_) + " ";
			str_sets += "rate:" + lx2s(it.second->rate_) + " ";
			str_sets += "factor:" + lx2s(it.second->factor_) + "]";
		});


		std::for_each(bets_.cbegin(), bets_.cend(),
			[&str_sets, this, &dat](const std::pair<std::string, std::vector<bet_item>>& it) {
			std::string str_bets;
			std::for_each(it.second.cbegin(), it.second.cend(),
				[&str_bets, it](const bet_item& bi) {
				str_bets += "[bet:" + lx2s(bi.bet_id_) + " count:" + lx2s(bi.cost_) +  "]";
			});

			str_bets += "[user balance:" + lx2s(warden_->add_user_balance(it.first, 0)) + "]";
		});

	}
}

void dice_controller::get_special_item_groups(std::map<int, dice_item_ptr>& ret)
{
	std::map<int, dice_item_ptr>& dices = *current_use_;
	
	std::accumulate(dices.begin(), dices.end(), 0, 
		[&ret](int val, const std::pair<int, dice_item_ptr>& p1)->int{
			if(p1.second->func_id_ == special_item_group){
				ret.insert(p1);
			}
			return 0;
	});
}

bool dice_controller::is_proper_balance(std::map<std::string, std::map<int, user_get_data>>& user_get, std::string modifer)
{
	__int64 usrb = -0x7FFFFFFF;
	if (!modifer.empty() && modifer != "all"){
		usrb = warden_->add_user_balance(modifer, 0);
	}

	__int64 willpay = std::accumulate(user_get.begin(), user_get.end(), 0,
		[](__int64 val, const std::pair<std::string, std::map<int, user_get_data>>& wins)->__int64 {
			__int64 willpay1 = std::accumulate(wins.second.begin(), wins.second.end(), 0, 
				[](__int64 val, const std::pair<int, user_get_data>& p)->__int64{
					return val += p.second.win_;
			});
			return val + willpay1;
	});

	return (willpay <= warden_->add_sysbalance(0) * conf_.max_sysbal_) &&  (willpay + usrb / 0.8) < 0;
}

void dice_controller::balance(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get)
{
	auto itf = (*current_use_).find(pid);
	const dice_item_ptr& di = itf->second;

	auto itp = bets_.begin();
	while (itp != bets_.end())
	{
		std::map<int, user_get_data>& wins = user_get[itp->first];
		std::vector<bet_item>& v = itp->second;
		for (unsigned i = 0; i < v.size(); i++)
		{
			//中奖了
			if (v[i].bet_id_ == di->bet_id_) {
				wins[di->bet_id_].win_ += di->factor_ * v[i].cost_;
				wins[di->bet_id_].bet_id_ = di->bet_id_;
				wins[di->bet_id_].pid_ = di->pid_;
			}
		}
		itp++;
	}
}

//结算组合奖
//从玩家下注列表中检查,有没有压组合奖,
//如果压了组合奖,看是否中奖.
void dice_controller::balance_group(const std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get)
{
	std::map<int, dice_item_ptr> group_items;
	get_special_item_groups(group_items);

	for (unsigned i = 0; i < dice_result.size(); i++)
	{
		int pid = dice_result[i];
		std::for_each(group_items.begin(), group_items.end(),
			[pid, this, &user_get](const std::pair<int, dice_item_ptr>& itgp){
				auto itf = std::find(itgp.second->win_for_.begin(), itgp.second->win_for_.end(), pid);
				//如果是本组合中奖了
				if (itf != itgp.second->win_for_.end()){
					//看用户有没有压这个团队
					std::for_each(bets_.begin(), bets_.end(), 
						[pid, itgp, &user_get](const std::pair<std::string, std::vector<bet_item>>& itubet){

						std::map<int, user_get_data>& uget = user_get[itubet.first];
						const std::vector<bet_item>& ubet = itubet.second;

						for (auto itbet = ubet.begin(); itbet != ubet.end(); ++itbet)
						{
							if (itbet->bet_id_ == itgp.second->bet_id_) {
								uget[itgp.second->bet_id_].win_ += itgp.second->factor_ * itbet->cost_;
								uget[itgp.second->bet_id_].bet_id_ = itgp.second->bet_id_;
								uget[itgp.second->bet_id_].pid_ = itgp.second->pid_;
								uget[itgp.second->bet_id_].pid2_ = pid;
							}
						}
					});
				}
		});
	}
}

void dice_controller::balance_all_bet_wins(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get)
{
	auto itp = bets_.begin();
	while (itp != bets_.end())
	{
		std::map<int, user_get_data>& wins = user_get[itp->first];
		std::vector<bet_item>& v = itp->second; 
		for (unsigned i = 0; i < v.size(); i++)
		{
			std::vector<dice_item_ptr> vitem;
			get_dice_by_betid((*current_use_), v[i].bet_id_, vitem);
			if (!vitem.empty()) {
				wins[v[i].bet_id_].win_ += vitem.front()->factor_ * v[i].cost_;
				wins[v[i].bet_id_].bet_id_ = v[i].bet_id_;
				wins[v[i].bet_id_].pid_ = pid;
			}
		}
		itp++;
	}
}

void dice_controller::balance_tractor(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get)
{
	int pid = do_dice(*current_use_);
	auto it = (*current_use_).find(pid);
	std::vector<int> wins;
	int i = 0;
	while (i < 5)
	{
		if (it == (*current_use_).end()) {
			it = (*current_use_).begin();
		}
		wins.push_back(it->first);
		i++;
		it++;
	}

	for (int i = 0; i < user_get.size(); i++) {
		balance(wins[i], user_get);
		dice_result.push_back(wins[i]);
	}
}

void dice_controller::balance_meteor(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get)
{
	std::vector<int> wins;
	wins.push_back(do_dice(*current_use_));
	wins.push_back(do_dice(*current_use_));
	wins.push_back(do_dice(*current_use_));

	for (int i = 0; i < user_get.size(); i++) {
		balance(wins[i], user_get);
		dice_result.push_back(wins[i]);
	}
}

void dice_controller::feedback_super_lotto(int pid, std::map<std::string, std::map<int, user_get_data>>& user_get)
{
	dice_item_ptr& di = (*current_use_)[pid];
	auto itp = user_get.begin();
	while (itp != user_get.end())
	{
		std::map<int, user_get_data>& wins = itp->second;
		std::vector<bet_item>& v = bets_[itp->first];
		for (unsigned i = 0; i < v.size(); i++)
		{
			if (v[i].bet_id_ == di->bet_id_) {
				wins[v[i].bet_id_].win_ += v[i].cost_;
				wins[v[i].bet_id_].bet_id_ += v[i].bet_id_;
				wins[v[i].bet_id_].pid_ += pid;
			}
		}
		itp++;
	}
}

void dice_controller::do_dice_dice(std::vector<int>& dice_result, std::map<std::string, std::map<int, user_get_data>>& user_get, int& adjust_type)
{
	int		bal = warden_->auto_balance();
	if (bal == balance_warden::auto_balance_prefer_system) {
		dice_perfer_system(dice_result, user_get, adjust_type, "all");
	}
	else if (bal == balance_warden::auto_balance_prefer_user) {
		dice_perfer_user(dice_result, user_get, adjust_type, "all");
	}
	else {
		if ((conf_.auto_balance_user_ || warden_->conf().user_stock_) && !conf_.user_mode_)	{
			std::string uid;
			int prefer = warden_->feedback_user(uid, bets_, (*current_use_), this);

			if (prefer == balance_warden::auto_balance_prefer_system) {
				dice_perfer_system(dice_result, user_get, adjust_type, uid);
			}
			//库存低于警戒线，就不开这个奖
			else if (prefer == balance_warden::auto_balance_prefer_user &&
				warden_->add_sysbalance(0) > warden_->conf().stock_lowcap_) {
				dice_perfer_user(dice_result, user_get, adjust_type, uid);
			}
			else{
				dice_normal(dice_result, user_get, adjust_type);
			}
		}
		else {
			dice_normal(dice_result, user_get, adjust_type);
		}

		if (warden_->conf().user_stock_){
			adjust_type = balance_warden::auto_balance_normal;
		}
	}
}

int dice_controller::find_best_pid_for_user(const std::map<int, __int64>& sumbet)
{
	if (sumbet.empty()){
		return special_item_invalid;
	}

	std::map<int, __int64> each_pid_willpay;
	evalute_pay((*current_use_), sumbet, each_pid_willpay);

	auto itmax = std::max_element(each_pid_willpay.begin(), each_pid_willpay.end(),
		[](std::pair<int, __int64> p1, std::pair<int, __int64> p2)->bool {
		return p1.second < p2.second;
	});

	//找到最大的下注项
	std::vector<std::pair<int, __int64>> v;
	std::for_each(each_pid_willpay.begin(), each_pid_willpay.end(), 
		[&v, itmax](const std::pair<int, __int64>& p)->void
	{
		if (p.second == itmax->second){
			v.push_back(p);
		}
	});

	if (!v.empty()){
		return v[rand_r(v.size() - 1)].first;
	}
	else{
		return special_item_invalid;
	}
}

int dice_controller::find_worse_pid_for_user(const std::map<int, __int64>& sumbet)
{
	if (sumbet.empty()){
		return special_item_invalid;
	}

	std::map<int, __int64> each_pid_will_pay;
	auto it = (*current_use_).begin();
	while (it != (*current_use_).end())
	{
		if (it->second->func_id_ == special_item_normal){
			each_pid_will_pay[it->first] = 0;
		}
		it++;
	}

	evalute_pay((*current_use_), sumbet, each_pid_will_pay);

	auto itmin = std::min_element(each_pid_will_pay.begin(), each_pid_will_pay.end(),
		[](const std::pair<int, __int64>& p1, const std::pair<int, __int64>& pmin)->bool {
		return pmin.second > p1.second;
	});
	
	//找到最小的下注项
	std::map<int, dice_item_ptr> v;
	std::for_each(each_pid_will_pay.begin(), each_pid_will_pay.end(), 
		[&v, itmin, this](const std::pair<int, __int64>& p)->void
	{
		if (p.second <= itmin->second){
			v.insert(std::make_pair(p.first, (*current_use_)[p.first]));
		}
	});

	if (v.empty()){
		return special_item_invalid;
	}
	else{
		return do_dice(v, true);
	}
}

dice_item::dice_item()
{
	max_count_ = 999;
	pid_ = special_item_invalid;
	rate_ = 0;
	factor_ = 0;
	bet_id_ = 0;
	group_id_ = 1;
	func_id_ = special_item_normal;
}

dice_controller_config::dice_controller_config()
{
	reset();
}

bool dice_controller_config::read_config(std::string file_name)
{
	reset();

	boost::property_tree::ptree root;

	try{
		boost::property_tree::read_xml(file_name, root);
	}
	catch (boost::property_tree::xml_parser_error& ec){
		std::cout << "read dice config xml error, file:"<< ec.filename() << " error:" << ec.message() << std::endl;
		return false;
	}

	boost::property_tree::ptree obj = root.get_child("config");
	json_msg_helper::read_value("auto_balance_user_", auto_balance_user_, obj);
	json_msg_helper::read_value("wincap_", wincap_, obj);
	json_msg_helper::read_value("losecap_", losecap_, obj);
	json_msg_helper::read_value("max_sysbal_", max_sysbal_, obj);
	json_msg_helper::read_value("feedback_factor_", feedback_factor_, obj);
	json_msg_helper::read_value("func_type_", func_type_, obj);
	
	int		cnt = 0;
	json_msg_helper::read_value("dice_count", cnt, obj);

	std::vector<std::map<int, dice_item_ptr>*> v;
	v.push_back(&dices_1);
	for (unsigned i = 0; i < v.size() && i < cnt; i++)
	{
		std::map<int, dice_item_ptr>& mdice = *v[i];

		boost::property_tree::ptree obj1 = obj.get_child("dices" + lx2s(i));
		int itemc = 0;
		json_msg_helper::read_value("item_count", itemc, obj1);
		for (int ii = 0 ; ii < itemc; ii++)
		{
			dice_item_ptr di(new dice_item());
			boost::property_tree::ptree di_obj = obj1.get_child("item" + lx2s(ii));
			json_msg_helper::read_value("bet_id_",		di->bet_id_, di_obj);
			json_msg_helper::read_value("factor_",		di->factor_, di_obj);
			json_msg_helper::read_value("max_count_",	di->max_count_, di_obj);
			json_msg_helper::read_value("pid_",			di->pid_, di_obj);
			json_msg_helper::read_value("rate_",		di->rate_, di_obj);
			json_msg_helper::read_value("rate_",		di->rate_, di_obj);
			json_msg_helper::read_value("name_",		di->name_, di_obj);
			json_msg_helper::read_value("groupid",		di->group_id_, di_obj);
			json_msg_helper::read_value("func_id_",		di->func_id_, di_obj);

			std::string str;
			json_msg_helper::read_value("win_for_",	str, di_obj);
			split_str(str, ",", di->win_for_, true);

			str.clear();
			json_msg_helper::read_value("exclude_", str, di_obj);
			split_str(str, ",", di->exclude_, true);

			mdice.insert(std::make_pair(di->pid_, di));
		}
	}

	std::string bfactors;
	json_msg_helper::read_value("bet_factors", bfactors, obj);
	read_bet_group(bfactors);
	return true;
}

bool dice_controller_config::save_config(std::string file_name)
{
	boost::property_tree::ptree root;
	boost::property_tree::ptree obj;

	std::vector<std::map<int, dice_item_ptr>*> v;
	v.push_back(&dices_1);

	json_msg_helper::write_value("dice_count", v.size(), obj);
	json_msg_helper::write_value("wincap_", wincap_, obj);
	json_msg_helper::write_value("losecap_", losecap_, obj);
	json_msg_helper::write_value("auto_balance_user_", auto_balance_user_, obj);
	json_msg_helper::write_value("max_sysbal_", max_sysbal_, obj);
	json_msg_helper::write_value("feedback_factor_", feedback_factor_, obj);
	json_msg_helper::write_value("func_type_", func_type_, obj);

	for(unsigned i = 0; i < v.size(); i++)
	{
		boost::property_tree::ptree obj1;

		json_msg_helper::write_value("item_count", (*v[i]).size(), obj1);
		std::map<int, dice_item_ptr>::iterator  it = (*v[i]).begin();
		int ii = 0;
		while (it != (*v[i]).end())
		{
			dice_item_ptr& di = it->second; it++;
			boost::property_tree::ptree di_obj;
			json_msg_helper::write_value("bet_id_",		di->bet_id_, di_obj);
			json_msg_helper::write_value("factor_",		di->factor_, di_obj);
			json_msg_helper::write_value("max_count_",	di->max_count_, di_obj);
			json_msg_helper::write_value("pid_",		di->pid_, di_obj);
			json_msg_helper::write_value("rate_",		di->rate_, di_obj);
			json_msg_helper::write_value("groupid",		di->group_id_, di_obj);
			json_msg_helper::write_value("name_",		di->name_, di_obj);
			json_msg_helper::write_value("func_id_",	di->func_id_, di_obj);
			std::string str = combin_str(di->win_for_, ",", true);
			json_msg_helper::write_value("win_for_", str, di_obj);
			obj1.add_child("item" + lx2s(ii++), di_obj);
		}
		obj.add_child("dices" + lx2s(i), obj1);
		obj1.clear();
	}

	json_msg_helper::write_value("bet_factors", get_bet_group(), obj);

	try{
		root.add_child("config", obj);
		boost::property_tree::write_xml(file_name, root);
	}
	catch (boost::property_tree::xml_parser_error& ec){
		std::cout << "write dice config xml error, file:"<< ec.filename() << " error:" << ec.message() << std::endl;
		return false;
	}

	return true;
}

bool dice_controller_config::read_bet_group(std::string bfactors)
{
	group_bet_factors_.clear();
	std::vector<std::string> vspl;
	split_str(bfactors, ",", vspl, true);
	if (vspl.size() % 2 != 0){
		return false;
	}

	for (unsigned i = 0; i < vspl.size(); i += 2)
	{
		group_bet_factors_[s2i<int>(vspl[i])] = s2i<float>(vspl[i + 1]);
	}
	return true;
}

std::string dice_controller_config::get_bet_group()
{
	std::string bfactors;
	auto it = group_bet_factors_.begin();
	while (it != group_bet_factors_.end())
	{
		bfactors += lx2s(it->first) + ",";
		bfactors += lx2s(it->second) + ",";
		it++;
	}
	return bfactors;
}

void dice_controller_config::reset()
{
	dices_1.clear();
	auto_balance_user_ = 0;
	func_type_ = func_type_normal;
	wincap_ = 50.0f;
	losecap_ = 1.3f;
	feedback_factor_ = 1.3f;
	max_sysbal_ = 0.4f;
}
