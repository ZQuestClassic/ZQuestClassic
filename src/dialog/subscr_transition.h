#ifndef ZC_DIALOG_SUBSCR_TRANS_H_
#define ZC_DIALOG_SUBSCR_TRANS_H_

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/button.h>
#include <initializer_list>
#include <string>
#include <string_view>
#include "subscr.h"

void call_subscrtransition_dlg(SubscrTransition& tr, std::string title = "Transition Editor");

class SubscrTransitionDialog: public GUI::Dialog<SubscrTransitionDialog>
{
public:
	enum class message
	{
		REFR_INFO, REFRESH, OK, CANCEL
	};

	SubscrTransitionDialog(SubscrTransition& src, std::string const& title);
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	std::shared_ptr<GUI::Window> window;
	int32_t args[sstrMAX][SUBSCR_TRANSITION_MAXARG];
	SubscrTransition local_trans;
	SubscrTransition* src_transition;
	std::string windowTitle;
	
	std::shared_ptr<GUI::Button> pastebtn;
	
	GUI::ListData list_sfx, list_dirs4;
	
	int32_t* cur_args() {return args[local_trans.type];}
	void refr_info();
};

#endif
