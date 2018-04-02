#include "dice_controller_slot.h"

enum GameIcon {
	GI_FUTOU = 0,     // 斧头
	GI_YINGQIANG,     // 银枪
	GI_DADAO,         // 大刀
	GI_LU,            // 鲁智深
	GI_LIN,           // 林冲
	GI_SONG,          // 宋江
	GI_TITIANXINGDAO, // 替天行道
	GI_ZHONGYITANG,   // 忠义堂
	GI_SHUIHUZHUAN,   // 水浒传

	GI_COUNT
};

struct GPOINT
{
	int x,y;
};
GameIcon g_icons_test[3][5] =
{
	{ GI_FUTOU,GI_FUTOU,GI_SHUIHUZHUAN,GI_FUTOU,GI_DADAO },
	{ GI_TITIANXINGDAO,GI_SHUIHUZHUAN,GI_ZHONGYITANG,GI_SHUIHUZHUAN,GI_LIN },
	{ GI_SHUIHUZHUAN,GI_ZHONGYITANG,GI_FUTOU,GI_SONG,GI_SHUIHUZHUAN }
};
GameIcon g_icons_none[9][3][5] =
{

	{
		{GI_FUTOU,GI_FUTOU,GI_YINGQIANG,GI_FUTOU,GI_DADAO},
		{GI_TITIANXINGDAO,GI_LU,GI_ZHONGYITANG,GI_LU,GI_LIN},
		{GI_FUTOU,GI_ZHONGYITANG,GI_FUTOU,GI_SONG,GI_SHUIHUZHUAN}
	},
	{
		{GI_YINGQIANG,GI_FUTOU,GI_SONG,GI_DADAO,GI_FUTOU},
		{GI_TITIANXINGDAO,GI_LU,GI_LU,GI_LU,GI_LIN},
		{GI_FUTOU,GI_ZHONGYITANG,GI_FUTOU,GI_TITIANXINGDAO,GI_SHUIHUZHUAN}
	},
	{
		{GI_SHUIHUZHUAN,GI_LIN,GI_SONG,GI_DADAO,GI_FUTOU},
		{GI_TITIANXINGDAO,GI_LU,GI_SONG,GI_LU,GI_DADAO},
		{GI_FUTOU,GI_ZHONGYITANG,GI_FUTOU,GI_TITIANXINGDAO,GI_SHUIHUZHUAN}
	},
	{
		{GI_FUTOU,GI_ZHONGYITANG,GI_FUTOU,GI_TITIANXINGDAO,GI_SHUIHUZHUAN},
		{GI_SHUIHUZHUAN,GI_LIN,GI_SONG,GI_DADAO,GI_FUTOU},
		{GI_TITIANXINGDAO,GI_LU,GI_SONG,GI_LU,GI_DADAO},
	},
	{
		{GI_TITIANXINGDAO,GI_LU,GI_SONG,GI_LU,GI_DADAO},
		{GI_FUTOU,GI_ZHONGYITANG,GI_FUTOU,GI_TITIANXINGDAO,GI_SHUIHUZHUAN},
		{GI_SHUIHUZHUAN,GI_LIN,GI_SONG,GI_DADAO,GI_FUTOU},
		},
		{
			{GI_ZHONGYITANG,GI_FUTOU,GI_YINGQIANG,GI_FUTOU,GI_LU},
			{GI_TITIANXINGDAO,GI_LU,GI_FUTOU,GI_LU,GI_LIN},
			{GI_YINGQIANG,GI_ZHONGYITANG,GI_FUTOU,GI_SONG,GI_LU}
		},
		{
			{GI_YINGQIANG,GI_ZHONGYITANG,GI_FUTOU,GI_SONG,GI_LU},
			{GI_ZHONGYITANG,GI_FUTOU,GI_YINGQIANG,GI_FUTOU,GI_LU},
			{GI_TITIANXINGDAO,GI_LU,GI_FUTOU,GI_LU,GI_LIN},
		},
		{
			{GI_LU,GI_SONG,GI_FUTOU,GI_SONG,GI_LU},
			{GI_ZHONGYITANG,GI_FUTOU,GI_YINGQIANG,GI_FUTOU,GI_LU},
			{GI_TITIANXINGDAO,GI_ZHONGYITANG,GI_FUTOU,GI_LU,GI_YINGQIANG},
			},
			{
				{GI_LIN,GI_SONG,GI_FUTOU,GI_SONG,GI_LU},
				{GI_SHUIHUZHUAN,GI_FUTOU,GI_YINGQIANG,GI_FUTOU,GI_LU},
				{GI_SONG,GI_ZHONGYITANG,GI_FUTOU,GI_LIN,GI_LU},
			},

};

// 派彩线具体参见"第一部分\说明界面\1.png"
const GPOINT kLines[9][5] = {
	{ {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4} },//---
	{ {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4} },//---
	{ {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4} },//___
	{ {0, 0}, {1, 1}, {2, 2}, {1, 3}, {0, 4} },//V
	{ {2, 0}, {1, 1}, {0, 2}, {1, 3}, {2, 4} },//^
	{ {0, 0}, {0, 1}, {1, 2}, {0, 3}, {0, 4} },//-V-
	{ {2, 0}, {2, 1}, {1, 2}, {2, 3}, {2, 4} },//-^-
	{ {1, 0}, {2, 1}, {2, 2}, {2, 3}, {1, 4} },
	{ {1, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 4} }
};

// 倍数
const int kIconsTimes5[GI_COUNT] = { 20, 40, 60, 100, 160, 200, 400, 1000, 2000 };
const int kIconsTimes4[GI_COUNT] = { 5, 10, 15, 20, 30, 40, 80, 200, 0 };
const int kIconsTimes3[GI_COUNT] = { 2, 3, 5, 7, 10, 15, 20, 50, 0 };
const int kIconsTimesAll[GI_COUNT] = {
	50, 100, 150, 250, 400, 500, 1000, 2500, 5000
};

//
const int kIconsTimesAllRate[GI_COUNT] = {
	20000, 10000, 6600, 4000, 2500, 100, 100, 100, 0
};


// 转盘中奖图标倍数
const int kRotateResultTimes[GI_COUNT] = { 2, 50, 10, 20, 50, 70, 100, 200, 0 };

int CalcResultTimes(const GameIcon game_result_icons[3][5], __int64* total_win_score,  bool* bonus_game) 
{
	__int64 bet_score = 1;
	int result_times = 0;
	*total_win_score = 0;
	*bonus_game = false;

	do {
		GameIcon temp_icons[3][5];
		memcpy(temp_icons, game_result_icons, sizeof(temp_icons));

		int icon_count[GI_COUNT] = {0};
		memset(icon_count, 0, sizeof(icon_count));
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 5; ++j) {
				++icon_count[temp_icons[i][j]];
			}
		}

		int icon = 0;
		// 全部是相同图标
		for (; icon < GI_COUNT; ++icon) {
			if (icon_count[icon] == 15) {
				result_times = kIconsTimesAll[icon];
				*total_win_score = kIconsTimesAll[icon] * bet_score * 9;
				break;
			}
		}
		if (result_times > 0)
			break;

		// 如果有5个水浒传在一条线上
		int pay_line_count = 0;
		if (icon_count[GI_SHUIHUZHUAN] >= 3) {
			for (int i = 0; i < 9; ++i) {
				pay_line_count = 0;
				for (int j = 0; j < 5; ++j) {
					if (temp_icons[kLines[i][j].x][kLines[i][j].y] != GI_SHUIHUZHUAN)
						break;
					++pay_line_count;
				}

				if (pay_line_count == 5) {
					result_times += kIconsTimes5[GI_SHUIHUZHUAN];
					*total_win_score += kIconsTimes5[GI_SHUIHUZHUAN] * bet_score;
				}

				if (pay_line_count >= 3)
					*bonus_game = true;
				else {
					pay_line_count = 0;
					for (int j = 4; j >= 0; --j) {
						if (temp_icons[kLines[i][j].x][kLines[i][j].y] != GI_SHUIHUZHUAN)
							break;
						++pay_line_count;
					}
					if (pay_line_count >= 3) {
						*bonus_game = true;
					}
				}
			}
		}

		bool have_pay_line = false;
		for (int i = 0; i < GI_COUNT - 1; ++i) {
			if (icon_count[i] + icon_count[GI_SHUIHUZHUAN] >= 3) {
				have_pay_line = true;
				break;
			}
		}
		if (!have_pay_line)
			break;

		// 普通图标+水浒传百搭
		for (int i = 0; i < GI_COUNT - 1; ++i) {
			if (icon_count[i] == 0) continue;
			for (int j = 0; j < 9; ++j) {
				// 必须靠边(靠左(row=0)或者靠右(row=4)
				if (temp_icons[kLines[j][0].x][kLines[j][0].y] !=
					static_cast<GameIcon>(i) &&
					temp_icons[kLines[j][4].x][kLines[j][4].y] !=
					static_cast<GameIcon>(i) &&
					temp_icons[kLines[j][0].x][kLines[j][0].y] != GI_SHUIHUZHUAN &&
					temp_icons[kLines[j][4].x][kLines[j][4].y] != GI_SHUIHUZHUAN)
					continue;
				pay_line_count = 0;
				bool all_shuihuzhuan = true;
				for (int k = 0; k < 5; ++k) {
					if (temp_icons[kLines[j][k].x][kLines[j][k].y] !=
						static_cast<GameIcon>(i) && 
						temp_icons[kLines[j][k].x][kLines[j][k].y] != GI_SHUIHUZHUAN)
						break;
					++pay_line_count;
					if (all_shuihuzhuan &&
						temp_icons[kLines[j][k].x][kLines[j][k].y] != GI_SHUIHUZHUAN)
						all_shuihuzhuan = false;
				}
				if (pay_line_count >= 3 && !all_shuihuzhuan) {  // 全部是水浒传不算
					if (pay_line_count == 5) {
						result_times += kIconsTimes5[i];
						*total_win_score += kIconsTimes5[i] * bet_score;
					} else if (pay_line_count == 4) {
						result_times += kIconsTimes4[i];
						*total_win_score += kIconsTimes4[i] * bet_score;
					} else {
						result_times += kIconsTimes3[i];
						*total_win_score += kIconsTimes3[i] * bet_score;
					}
				} else {
					// 从右向左再判断一次
					pay_line_count = 0;
					bool all_shuihuzhuan = true;
					for (int k = 4; k >= 0; --k) {
						if (temp_icons[kLines[j][k].x][kLines[j][k].y] !=
							static_cast<GameIcon>(i) && 
							temp_icons[kLines[j][k].x][kLines[j][k].y] != GI_SHUIHUZHUAN)
							break;
						++pay_line_count;
						if (all_shuihuzhuan &&
							temp_icons[kLines[j][k].x][kLines[j][k].y] != GI_SHUIHUZHUAN)
							all_shuihuzhuan = false;
					}
					if (pay_line_count >= 3 && !all_shuihuzhuan) {  // 全部是水浒传不算
						if (pay_line_count == 5) {
							result_times += kIconsTimes5[i];
							*total_win_score += kIconsTimes5[i] * bet_score;
						} else if (pay_line_count == 4) {
							result_times += kIconsTimes4[i];
							*total_win_score += kIconsTimes4[i] * bet_score;
						} else {
							result_times += kIconsTimes3[i];
							*total_win_score += kIconsTimes3[i] * bet_score;
						}
					}
				}
			}
		}

		// 全部都是人物
		if (icon_count[GI_LU] + icon_count[GI_LIN] + icon_count[GI_SONG] == 15) {
			result_times += 50;
			*total_win_score += 50 * bet_score * 9;
		}

		// 全部都是武器
		if (icon_count[GI_FUTOU] + icon_count[GI_YINGQIANG] + icon_count[GI_DADAO] == 15) {
			result_times += 15;
			*total_win_score += 15 * bet_score * 9;
		}
	} while (0);

	return result_times;
}

void generate_dice_items(std::map<int, dice_item_ptr>& vid)
{
	static std::map<int, dice_item_ptr> gvdi;
	if (gvdi.empty()){
		//动态生成基础项 4200
		for (int i = 0; i < GI_LU; i++)
		{
			dice_item_ptr di(new dice_item());
			di->pid_ = i;
			di->rate_ = 1500;
			gvdi[i] = di;
		}

		//4800
		for (int i = GI_LU; i <= GI_TITIANXINGDAO; i++ )
		{
			dice_item_ptr di(new dice_item());
			di->pid_ = i;
			di->rate_ = 1200;
			gvdi[i] = di;
		}

		//1000
		for (int i = GI_ZHONGYITANG; i <= GI_SHUIHUZHUAN; i++ )
		{
			dice_item_ptr di(new dice_item());
			di->pid_ = i;
			di->rate_ = 500;
			gvdi[i] = di;
		}
	}
	vid = gvdi;
}

dice_controller_slot::dice_controller_slot()
{
	line_count_ = 9;
}

void dice_controller_slot::dice_dice(std::vector<dice_result_data>& dice_result, int group /*= -1*/)
{
	do_dice_dice(conf_.dices_1, dice_result);

	warden_->update_warden(bets_, dice_result, user_data_, this);
	warden_->next_turn(dice_result);
}

void dice_controller_slot::do_dice_dice(
	const std::map<int, dice_item_ptr>& vid,
	std::vector<dice_result_data>& dice_result, 
	int bal)
{
	__int64 winc = 0;
	bool bonus = false;

	dice_result_data dr;
	GameIcon game_result_icons[3][5];
	if (bal == -1){
		bal = warden_->auto_balance();
		//如果系统库存正常,看个人库存
		if (bal == balance_warden::auto_balance_normal && 
			(conf_.auto_balance_user_ || warden_->conf().user_stock_)) {
			std::string uid = bets_.begin()->first;
			__int64 lb = warden_->add_user_balance(uid, 0);
			if (lb < conf_.losecap_ || warden_->conf().user_stock_) {
				int r = rand_r(10000);
				if (r < 4000) {
					bal = balance_warden::auto_balance_prefer_user;
				}
			}
			else if (lb > conf_.wincap_) {
				int r = rand_r(10000);
				if (r < 2000) {
					bal = balance_warden::auto_balance_prefer_system;
				}
			}
		}
	}

	int rnd = rand_r(1000000);
	rnd *= 1.3;
	int allsame = -1;
	for (int i = 0; i < GI_COUNT; i++)
	{
		if (rnd >= kIconsTimesAllRate[i] / (4 * line_count_)){
			rnd -= kIconsTimesAllRate[i] / (4 * line_count_);
		}
		else {
			allsame = i;
			break;
		}
	}

	if (allsame >= 0){
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				game_result_icons[i][j] = (GameIcon)allsame;
				dr.dice_result_.push_back(game_result_icons[i][j]);
			}
		}
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				game_result_icons[i][j] = (GameIcon)do_dice(vid, false);
				dr.dice_result_.push_back(game_result_icons[i][j]);
			}
		}
	}
	
	__int64 factor = CalcResultTimes(game_result_icons, &winc, &bonus);

	__int64 will_win = 0;
	auto itp = bets_.begin();
	while (itp != bets_.end())
	{
		std::map<int, user_get_data>& wins = dr.user_get_[itp->first];
		std::vector<bet_item>& v = itp->second;
		for (unsigned i = 0; i < v.size(); i++)
		{
			wins[0].win_ += winc * (v[i].cost_ / line_count_);
			wins[0].pid_ = winc;
			wins[0].pid2_ = bonus;
			will_win = wins[0].win_;
		}
		itp++;
	}
	
	dr.adjust_type_ = bal;

	__int64 cap = conf_.banker_deposit_ * conf_.max_sysbal_;
	if ((bal == balance_warden::auto_balance_prefer_system && will_win > 0) || will_win >= cap) {
		dr.clear();
		int indx = rand_r(8);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				game_result_icons[i][j] = g_icons_none[indx][i][j];
				dr.dice_result_.push_back(g_icons_none[indx][i][j]);
			}
		}
		__int64 factor = CalcResultTimes(game_result_icons, &winc, &bonus);
		if (factor > 0)	{
			std::cout << "error!!! line " << indx << " should be factor 0 but is factor "<< factor << std::endl;
		}

	}
	else if (bal == balance_warden::auto_balance_prefer_user){
		if (winc == 0 || winc >= 1000){
			do_dice_dice(vid, dice_result, bal);
			return;
		}
	}
	dice_result.push_back(dr);
}

void dice_controller_slot::build_default_dice_items()
{
	std::map<int, dice_item_ptr> vid;
	generate_dice_items(vid);
	conf_.dices_1 = vid;
}

