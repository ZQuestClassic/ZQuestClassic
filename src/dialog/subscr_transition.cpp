#include "subscr_transition.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>
#include <sstream>
#include <fmt/format.h>
#include "new_subscr.h"
#include "zq/zq_subscr.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"

static SubscrTransition copytrans;
static bool copied_trans = false;
void call_subscrtransition_dlg(SubscrTransition& tr, std::string title)
{
	SubscrTransitionDialog(tr, title).show();
}

SubscrTransitionDialog::SubscrTransitionDialog(SubscrTransition& src, std::string const& title) :
	local_trans(src), src_transition(&src), windowTitle(title),
	list_sfx(GUI::ZCListData::sfxnames(true))
{
	memset(args,0,sizeof(args));
	args[sstrSLIDE][1] = 40000;
	args[sstrPIXEL][1] = 45;
	auto* curarg = cur_args();
	for(int q = 0; q < SUBSCR_TRANSITION_MAXARG; ++q)
		curarg[q] = local_trans.arg[q];
}

static const GUI::ListData list_trtype
{
	{ "Instant", sstrINSTANT, "Instantaneously transitions to the new page." },
	{ "Slide", sstrSLIDE, "Old page slides out in the specified direction, new page slides"
		" in from the opposite side behind it." },
	{ "Pixellate", sstrPIXEL, "Pixellate out from one page to the other."
		" Note that this pattern is NOT random." },
};
static const GUI::ListData list_4dirs
{
	{ "Up", 0 },
	{ "Down", 1 },
	{ "Left", 2 },
	{ "Right", 3 }
};

std::shared_ptr<GUI::Widget> SubscrTransitionDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Frame> fr;
	window = Window(
		title = windowTitle,
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				DummyWidget(),
				Rows<3>(
					Label(text = "SFX:", hAlign = 1.0),
					DropDownList(data = list_sfx,
						fitParent = true,
						selectedValue = local_trans.tr_sfx,
						onSelectFunc = [&](int32_t val)
						{
							local_trans.tr_sfx = val;
						}),
					INFOBTN("SFX to play when the transition starts."),
					Label(text = "Type:", hAlign = 1.0),
					DropDownList(data = list_trtype,
						fitParent = true,
						selectedValue = local_trans.type,
						onSelectionChanged = message::REFRESH,
						onSelectFunc = [&](int32_t val)
						{
							local_trans.type = val;
						}),
					INFOBTN("The type of transition")
				),
				Rows<2>(
					Checkbox(_EX_RBOX,
						text = "Don't Hide Selector",
						checked = local_trans.flags & SUBSCR_TRANS_NOHIDESELECTOR,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_trans.flags,SUBSCR_TRANS_NOHIDESELECTOR,state);
						}),
					INFOBTN("Don't hide the selector cursor during the animation")
				),
				fr = Frame(title = list_trtype.findText(local_trans.type),
					info = list_trtype.findInfo(local_trans.type),
					minheight = 250_px, minwidth = 400_px)
			),
			Rows<2>(
				Button(
					text = "&Copy",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						copytrans = local_trans;
						auto* curarg = cur_args();
						for(int q = 0; q < SUBSCR_TRANSITION_MAXARG; ++q)
							copytrans.arg[q] = curarg[q];
						copied_trans = true;
						pastebtn->setDisabled(false);
					}),
				pastebtn = Button(
					text = "Paste",
					topPadding = 0.5_em,
					minwidth = 90_px,
					disabled = !copied_trans,
					onClick = message::REFRESH,
					onPressFunc = [&]()
					{
						if(copied_trans)
						{
							local_trans = copytrans;
							auto* curarg = cur_args();
							for(int q = 0; q < SUBSCR_TRANSITION_MAXARG; ++q)
								curarg[q] = copytrans.arg[q];
						}
					}),
				Button(
					text = "&OK",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::OK,
					focused = true),
				Button(
					text = "Cancel",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	switch(local_trans.type)
	{
		case sstrINSTANT:
			fr->setContent(Label(text = "No Arguments"));
			break;
		case sstrSLIDE:
			fr->setContent(Rows<3>(
				Label(text = "Direction:", hAlign = 1.0),
				DropDownList(data = list_4dirs,
					fitParent = true,
					selectedValue = args[sstrSLIDE][0],
					onSelectFunc = [&](int32_t val)
					{
						args[sstrSLIDE][0] = val;
					}),
				INFOBTN("The direction the pages slide in"),
				Label(text = "Speed:", hAlign = 1.0),
				TextField(maxLength = 13, type = GUI::TextField::type::NOSWAP_ZSINT,
					hAlign = 1.0, val = args[sstrSLIDE][1],
					swap_type = nswapDEC, fitParent = true,
					low = 5000, high = 9990000,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						args[sstrSLIDE][1] = val;
					}),
				INFOBTN("The speed, in pixels per frame, of the sliding.")
			));
			break;
		case sstrPIXEL:
			fr->setContent(Rows<3>(
				Checkbox(colSpan = 2,
					text = "Invert", _EX_RBOX,
					checked = args[sstrPIXEL][0] & TR_PIXELATE_INVERT,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(args[sstrPIXEL][0],TR_PIXELATE_INVERT,state);
					}),
				INFOBTN("Invert the pixellation pattern"),
				Label(text = "Duration:", hAlign = 1.0),
				TextField(type = GUI::TextField::type::INT_DECIMAL,
					val = args[sstrPIXEL][1], fitParent = true,
					low = 1, high = 60*60,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						args[sstrPIXEL][1] = val;
					}),
				INFOBTN("The duration, in frames, of the transition."),
				Label(text = "Pattern XOffset:", hAlign = 1.0),
				TextField(type = GUI::TextField::type::INT_DECIMAL,
					val = args[sstrPIXEL][2], fitParent = true,
					low = -99999, high = 99999,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						args[sstrPIXEL][2] = val;
					}),
				INFOBTN("Apply an X-Offset to the pixellation pattern."),
				Label(text = "Pattern YOffset:", hAlign = 1.0),
				TextField(type = GUI::TextField::type::INT_DECIMAL,
					val = args[sstrPIXEL][3], fitParent = true,
					low = -99999, high = 99999,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						args[sstrPIXEL][3] = val;
					}),
				INFOBTN("Apply a Y-Offset to the pixellation pattern.")
			));
			break;
	}
	
	return window;
}

bool SubscrTransitionDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::REFR_INFO:
			break;
		case message::REFRESH:
			rerun_dlg = true;
			return true;
		case message::OK:
		{
			auto* curarg = cur_args();
			for(int q = 0; q < SUBSCR_TRANSITION_MAXARG; ++q)
				local_trans.arg[q] = curarg[q];
			*src_transition = local_trans;
			return true;
		}
		case message::CANCEL:
			return true;
	}
	return false;
}

