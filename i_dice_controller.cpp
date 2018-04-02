#include "i_dice_controller.h"

extern void	get_dice_by_betid(const std::map<int, dice_item_ptr>& dice, int bet_id, std::vector<dice_item_ptr>& ret);
//得到玩家的下注总额
static void		sum_bet(
	const i_dice_controller* dc,
	const std::map<std::string, std::vector<bet_item>>& bets,
	std::string uid,
	std::map<int, __int64>& ret, 
	bool detail)
{
	std::vector<bet_item> vb;
	if (uid == "" || uid == "all") {
		auto it = bets.begin();
		while (it != bets.end())
		{
			if (dc == nullptr){
				vb.insert(vb.end(), it->second.begin(), it->second.end());
			}
			else {
				auto itf = dc->user_data_.find(it->first);
				//排除机器人(itf->second == 1)
				if (itf != dc->user_data_.end() && itf->second == 0) {
					vb.insert(vb.end(), it->second.begin(), it->second.end());
				}
			}
			it++;
		}
	}
	else {
		auto it = bets.find(uid);
		if (it != bets.end()) {
			vb = it->second;
		}
	}

	//每个压注项一共压了多少钱
	for (unsigned i = 0; i < vb.size(); i++)
	{
		if (detail) {
			ret[vb[i].bet_id_] += vb[i].cost_;
		}
		else {
			ret[0] += vb[i].cost_;
		}
	}
}

bool i_dice_controller::config(dice_controller_config& conf, balance_warden* war)
{
	if (!war){
		return false;
	}
	conf_ = conf;

	warden_ = war;

	old_banker_deposit_ = conf.banker_deposit_;

	return true;
}

bool i_dice_controller::set_bet(std::string uid, int is_bot, int bet_id, __int64 cnt, int groupid, bool istest /*= false*/, __int64 cap /*= 0xffffffff*/)
{
	std::map<int, __int64> ret;
	sum_bet(this, bets_, uid, ret, true);
	if (ret[bet_id] + cnt > cap){
		return false;
	}

	__int64 dep = conf_.banker_deposit_;
	ret.clear();
	sum_bet(this, bets_, "all", ret, false);
	if (!ret.empty()){
		dep += ret.begin()->second;
	}

	ret.clear();
	sum_bet(this, bets_, "all", ret, true);
	std::vector<dice_item_ptr> vitem;

	if (groupid >= 0){
		std::map<int, dice_item_ptr> md;
		std::for_each(conf_.dices_1.begin(), conf_.dices_1.end(),
			[&md, groupid](const std::pair<int, dice_item_ptr>& it) {
				if ((*it.second).group_id_ == groupid) {
					md[it.first] = it.second;
				}
		});
		get_dice_by_betid(md, bet_id, vitem);
	}
	else{
		get_dice_by_betid(conf_.dices_1, bet_id, vitem);
	}

	if (vitem.empty()){
		return false;
	}
	else{
		//应用下注权值
		auto it_gbf = conf_.group_bet_factors_.find(vitem.front()->group_id_);
		if (it_gbf != conf_.group_bet_factors_.end())
		{
			cnt = ceil(cnt * it_gbf->second);
		}

		if (!ret.empty()){
			ret[bet_id] = ret[bet_id] * vitem.front()->factor_;
		}
		else {
			ret[bet_id] = 0;
		}

		ret[bet_id] += cnt * vitem.front()->factor_;

// 		if (ret[bet_id] > conf_.banker_deposit_){
// 			return false;
// 		}
	}

	if (!istest){
		std::vector<bet_item>& v = bets_[uid];
		bet_item bi;
		bi.bet_id_ = bet_id;
		bi.cost_ = cnt;
		v.push_back(bi);
		user_data_[uid] = is_bot;
		warden_->user_pay(uid, cnt);
	}
	return true;
}

bool i_dice_controller::clear_bets(std::string uid)
{
	if (bets_.find(uid) == bets_.end()) {
		return false;
	}
	else {
		bets_.erase(uid);
		user_data_.erase(uid);
		warden_->clear_user_pay(uid);
		return true;
	}
}

int i_dice_controller::do_dice(const std::map<int, dice_item_ptr>& dices, bool sumrate)
{
	int pid = special_item_invalid;
	int rnd = rand_r(10000);
	dice_point_ = rnd;

	if (sumrate){
		int sumr = std::accumulate(dices.begin(), dices.end(), 0,
			[](int n, const std::pair<int, dice_item_ptr>& it)->int{
				return n + it.second->rate_;
		});
		rnd = rand_r(sumr);
	}

	auto it = dices.begin();
	while (it != dices.end())
	{
		const dice_item_ptr& di = it->second; it++;

		if (di->func_id_ == special_item_group)
			continue;

		if (rnd <= di->rate_) {
			pid = di->pid_;
			break;
		}
		else {
			rnd -= di->rate_;
		}
	}
	return pid;
}

void i_dice_controller::reset_user_data()
{
	bets_.clear();
	user_data_.clear();
}

void dice_result_data::clear()
{
	dice_result_.clear();
	user_get_.clear();
	adjust_type_ = balance_warden::auto_balance_normal;
	group_ = -1;
}

dice_result_data::dice_result_data()
{
	clear();
}
