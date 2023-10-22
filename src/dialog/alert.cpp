#include "alert.h"
#include <gui/builder.h>
#include <utility>

void info_dsa(std::string const& title, std::string const& text, std::string const& dsastr)
{
	if(!zc_get_config("dsa",dsastr.c_str(),0))
	{
		AlertDialog(title, text,
			[&](bool ret,bool dsa)
			{
				if(dsa)
				{
					zc_set_config("dsa",dsastr.c_str(),1);
				}
			},
			"OK","",
			0,false, //timeout - none
			true //"Don't show this again"
		).show();
	}
}

AlertDialog::AlertDialog(std::string title, std::string text, std::function<void(bool,bool)> onEnd, std::string truebtn, std::string falsebtn, uint32_t timeout, bool default_ret, bool dontshow):
	InfoDialog(title,text), truebtn(truebtn), falsebtn(falsebtn), timer(0), timeout(timeout), default_ret(default_ret), dontshowagain(dontshow), onEnd(onEnd)
{}

AlertDialog::AlertDialog(std::string title, std::vector<std::string_view> lines, std::function<void(bool,bool)> onEnd, std::string truebtn, std::string falsebtn, uint32_t timeout, bool default_ret, bool dontshow):
	InfoDialog(title,lines), truebtn(truebtn), falsebtn(falsebtn), timer(0), timeout(timeout), default_ret(default_ret), dontshowagain(dontshow), onEnd(onEnd)
{}

int32_t AlertDialog::alert_on_tick()
{
	if(timeout)
	{
		if(++timer > timeout)
		{
			onEnd(default_ret,dontshowagain);
			return ONTICK_EXIT;
		}
	}
	return ONTICK_CONTINUE;
}

std::shared_ptr<GUI::Widget> AlertDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	bool dsa = dontshowagain;
	dontshowagain = false;
	
	std::shared_ptr<GUI::Widget> trueb = DummyWidget();
	std::shared_ptr<GUI::Widget> falseb = DummyWidget();
	if (truebtn.size())
		trueb = Button(
			text = truebtn,
			minwidth = 90_px,
			onClick = message::OK,
			focused = true
		);
	if (falsebtn.size())
		falseb = Button(
			text = falsebtn,
			minwidth = 90_px,
			onClick = message::CANCEL
		);
	
	return Window(
		title = std::move(dlgTitle),
		onClose = message::CANCEL,
		use_vsync = true,
		onTick = [&](){return alert_on_tick();},
		hPadding = 0_px,
		Column(
			hPadding = 0_px, 
			Label(noHLine = true,
				hPadding = 2_em,
				maxLines = 20,
				maxwidth = Size::pixels(zq_screen_w)-12_px-5_em,
				textAlign = 1,
				text = std::move(dlgText)),
			Checkbox(visible = dsa,
				text = "Don't show this message again",
				checked = false,
				onToggleFunc = [&](bool state)
				{
					dontshowagain = state;
				}
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				trueb,
				falseb
			)
		)
	);
}

bool AlertDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			onEnd(true,dontshowagain);
			return true;
		case message::CANCEL:
			onEnd(false,dontshowagain);
			return true;
	}
	return false;
}
