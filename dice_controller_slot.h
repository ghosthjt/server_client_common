#pragma once
#include "i_dice_controller.h"

class dice_controller_slot : public i_dice_controller
{
public:
	dice_controller_slot();
	int			line_count_;
	virtual void	dice_dice(
		std::vector<dice_result_data>& dice_result, 
		int group = -1);

protected:
	void	do_dice_dice(
		const std::map<int, dice_item_ptr>& vid, 
		std::vector<dice_result_data>& dice_result,
		int bal = -1);

	virtual void build_default_dice_items();
};