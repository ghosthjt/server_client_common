#pragma once
#include "i_controller.h"
#include "i_transaction.h"
#include <map>

class transaction_controller : public i_controller
{
public:
	static transaction_controller* get_instance();
	static void	free_instance();

	virtual int		start() override;
	virtual int		step()	override;
	virtual int		stop()	override;

	void	start_transaction(std::string key, trans_ptr tr);

protected:
	std::map<std::string, trans_ptr> trans_;
};