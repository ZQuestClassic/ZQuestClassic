#ifndef ZC_DIALOG_SUBSCR_LISTS_EDIT_H_
#define ZC_DIALOG_SUBSCR_LISTS_EDIT_H_

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/button.h>
#include <initializer_list>
#include <string>
#include <string_view>
#include "subscr.h"

void call_subscr_listedit_dlg();

class SubscrListEditDialog: public GUI::Dialog<SubscrListEditDialog>
{
public:
	enum class message
	{
		REFR_INFO, REFRESH, DONE, EDIT0, EDIT1, EDIT2, DEL0, DEL1, DEL2
	};

	SubscrListEditDialog() = default;
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	std::shared_ptr<GUI::Window> window;
	
	GUI::ListData subscr_lists[sstMAX];
	size_t sel_inds[sstMAX];
	int copy_inds[sstMAX] = {-1,-1,-1};
	
	void rclick_menu(size_t cur_type, int mx, int my);
};

#endif
