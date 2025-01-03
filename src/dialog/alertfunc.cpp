#include "alertfunc.h"
#include <gui/builder.h>
#include <utility>
#include <cstdarg>
#include <gui/size.h>
extern int32_t zq_screen_w;

AlertFuncDialog::AlertFuncDialog(string const& title, string const& text, string info)
	: InfoDialog(title,text), didend(false), helptxt(info), chosen_ptr(nullptr)
{}

AlertFuncDialog::AlertFuncDialog(string const& title, vector<string> const& lines, string info)
	: InfoDialog(title,lines), didend(false), helptxt(info), chosen_ptr(nullptr)
{}

AlertFuncDialog& AlertFuncDialog::add_buttons(uint32_t focused_button,
	std::initializer_list<string> buttonNames,
	std::initializer_list<std::function<bool()>> buttonProcs)
{
	initButtons(buttonNames, buttonProcs, focused_button);
	return *this;
}
AlertFuncDialog& AlertFuncDialog::add_buttons(uint32_t focused_button,
	std::initializer_list<string> buttonNames, int& chosen)
{
	std::vector<std::function<bool()>> buttonProcs;
	for(size_t q = 0; q < buttonNames.size(); ++q)
		buttonProcs.emplace_back([&chosen,q](){chosen = int(q); return true;});
	initButtons(buttonNames, buttonProcs, focused_button);
	chosen_ptr = &chosen;
	return *this;
}

std::shared_ptr<GUI::Widget> AlertFuncDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if(buttons.empty())
	{
		buttons.push_back(
			Button(
				text = "OK",
				minwidth = 90_px,
				onClick = message::OK,
				focused = true
			));
	}
	
	auto sz = buttons.size();
	switch(sz)
	{
		case 1: case 2: case 3:
			buttonRow = Row(topPadding = 0.5_em,spacing = 10_px);
			break;
		default:
		{
			size_t s = (sz+1)/2;
			buttonRow = GUI::Internal::makeRows(s);
			buttonRow->setTopPadding(0.5_em);
			//buttonRow->setSpacing(10_px); // This breaks the sizing of some buttons.... why??
			break;
		}
	}
	for(std::shared_ptr<GUI::Button>& btn : buttons)
	{
		buttonRow->add(btn);
	}
	
	std::shared_ptr<GUI::Window> window = Window(
		title = std::move(d_title),
		info = helptxt,
		onClose = message::CANCEL,
		Column(
			build_text(),
			buttonRow
		)
	);
	return window;
}

void AlertFuncDialog::initButtons(std::initializer_list<string> buttonNames,
	std::vector<std::function<bool()>> buttonProcs,
	uint32_t focused_button)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	auto numButtons = buttonNames.size();
	assert(numButtons == buttonProcs.size());
	
	if(!numButtons) return;
	
	auto nameIter = buttonNames.begin();
	auto procIter = buttonProcs.begin();
	for(uint32_t q = 0; q < numButtons; ++q)
	{
		string const& btntext = *(nameIter++);
		std::function<bool()> const& func = *(procIter++);
		if(func)
		{
			buttons.push_back(
				Button(
					text = btntext,
					minwidth = 90_px, fitParent = true,
					onPressFunc = [&,func]()
					{
						BITMAP* tmp = create_bitmap_ex(8, screen->w, screen->h);
						blit(screen,tmp,0,0,0,0,screen->w,screen->h);
						didend = func();
						blit(tmp,screen,0,0,0,0,screen->w,screen->h);
						destroy_bitmap(tmp);
					},
					onClick = message::BTN,
					focused = (q==focused_button)
				));
		}
		else
		{
			buttons.push_back(
				Button(
					text = btntext,
					minwidth = 90_px, fitParent = true,
					onClick = message::OK,
					focused = (q==focused_button)
				));
		}
	}
}

bool AlertFuncDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			return true;
		case message::CANCEL:
			if(chosen_ptr)
				*chosen_ptr = -1;
			return true;
		case message::BTN:
			return didend;
	}
	return false;
}

