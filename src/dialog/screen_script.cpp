#include "screen_script.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "alert.h"
#include "zc_list_data.h"

extern script_data *screenscripts[NUMSCRIPTSCREEN];
void call_screenscript_dialog(mapscr* scr)
{
	if(!scr)
		scr = Map.CurrScr();
	ScreenScriptDialog(scr).show();
}

ScreenScriptDialog::ScreenScriptDialog(mapscr* scr) :
	thescr(scr), local_scr(*scr),
	list_screenscript(GUI::ZCListData::screen_script())
{}

std::shared_ptr<GUI::Widget> ScreenScriptDialog::SCREEN_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_initd[index]).show();
			}),
		tf_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_scr.screeninitd[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_scr.screeninitd[index] = val;
			})
	);
}

void ScreenScriptDialog::refreshScript()
{
	std::string label[8], help[8];
	for(auto q = 0; q < 8; ++q)
	{
		label[q] = "InitD["+std::to_string(q)+"]";
	}
	if(local_scr.script)
	{
		zasm_meta const& meta = screenscripts[local_scr.script]->meta;
		for(size_t q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				label[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				help[q] = meta.initd_help[q];
		}
		
		for(auto q = 0; q < 8; ++q)
		{
			if(unsigned(meta.initd_type[q]) < nswapMAX)
				tf_initd[q]->setSwapType(meta.initd_type[q]);
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
		{
			tf_initd[q]->setSwapType(nswapDEC);
		}
	}
	for(auto q = 0; q < 8; ++q)
	{
		l_initds[q]->setText(label[q]);
		h_initd[q] = help[q];
		ib_initds[q]->setDisabled(help[q].empty());
	}
}

std::shared_ptr<GUI::Widget> ScreenScriptDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	window = Window(
		title = "Screen Script",
		onClose = message::CANCEL,
		Column(
			Row(
				Column(
					SCREEN_INITD(0),
					SCREEN_INITD(1),
					SCREEN_INITD(2),
					SCREEN_INITD(3),
					SCREEN_INITD(4),
					SCREEN_INITD(5),
					SCREEN_INITD(6),
					SCREEN_INITD(7)
				),
				Column(
					padding = 0_px, fitParent = true,
					Rows<2>(vAlign = 0.0,
						SCRIPT_LIST_PROC("Action Script:", list_screenscript, local_scr.script, refreshScript)
					),
					Checkbox(
						hAlign = 0.0,
						checked = local_scr.preloadscript,
						text = "Run On Screen Init",
						onToggleFunc = [&](bool state)
						{
							local_scr.preloadscript = state ? 1 : 0;
						}
					)
				)
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	refreshScript();
	return window;
}

bool ScreenScriptDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			*thescr = local_scr;
			saved = false;
			[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
	}
	return false;
}

