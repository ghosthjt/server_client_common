#pragma once
#include "afxwin.h"
class		msgloop
{
public:
	msgloop()
	{
		exit_ = false;
		result_ = -100;
	}

	void			exit(int ret)
	{
		exit_ = true;
		result_ = ret;

	}

	int			loop()
	{
		exit_ = false;
		result_ = -100;
		while (!exit_)
		{
			MSG msg;
			do
			{
				if (!AfxPumpMessage()){
					::PostQuitMessage(0);
					return -1;
				}

			} while (::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));
		}
		return result_;
	}
private:
	int		result_;
	bool	exit_;
};
