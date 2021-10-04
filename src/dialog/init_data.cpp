#include "init_data.h"
#include "info.h"
#include "gui/use_size.h"
#include <gui/builder.h>

extern bool saved;
void call_init_dlg(zinitdata& sourcezinit)
{
    InitDataDialog(sourcezinit,
        [&sourcezinit](zinitdata const& other)
		{
			saved = false;
			sourcezinit = other;
		}).show();
}

InitDataDialog::InitDataDialog(zinitdata const& start, std::function<void(zinitdata const&)> setVals):
	local_zinit(start), setVals(setVals)
{}

#define SBOMB_RATIO (local_zinit.nBombmax / (local_zinit.bomb_ratio > 0 ? local_zinit.bomb_ratio : 4))

#define BYTE_FIELD(member) \
TextField(maxLength = 3, type = GUI::TextField::type::INT_DECIMAL, \
	high = 255, val = local_zinit.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
	{ \
		local_zinit.member = val; \
	})

#define WORD_FIELD(member) \
TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL, \
	high = 65535, val = local_zinit.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
	{ \
		local_zinit.member = val; \
	})

#define COUNTER_FRAME(name, field1, field2) \
Rows<2>( \
	framed = true, frameText = name, hAlign = 0.0, \
	margins = 2_spx, \
	Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"), \
	Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"), \
	field1, \
	field2 \
)

std::shared_ptr<GUI::Widget> InitDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Widget> counterTabs[4];
	//Varying tabpanels based on small/large mode...
	if(is_large)
	{
		counterTabs[0] = TabRef(name = "Engine", Rows<2>(hAlign = 0.0, vAlign = 0.0,
			Rows<2>(
				framed = true, frameText = "Bombs", hAlign = 0.0,
				margins = 2_spx,
				Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
				Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
				WORD_FIELD(nBombs),
				TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
					high = 65535, val = local_zinit.nBombmax,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
					{
						local_zinit.nBombmax = val;
						sBombMax->setVal(SBOMB_RATIO);
					})
			),
			Rows<3>(
				framed = true, frameText = "Super Bombs", hAlign = 0.0,
				margins = 2_spx,
				Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
				Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
				Label(hAlign = 1.0, bottomPadding = 0_px, text = "Ratio"),
				WORD_FIELD(nSbombs),
				sBombMax = TextField(
					maxLength = 5,
					type = GUI::TextField::type::INT_DECIMAL,
					val = SBOMB_RATIO,
					disabled = true
				),
				TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
					high = 255, val = local_zinit.bomb_ratio,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
					{
						local_zinit.bomb_ratio = val;
						sBombMax->setVal(SBOMB_RATIO);
					})
			),
			COUNTER_FRAME("Arrows", WORD_FIELD(nArrows), WORD_FIELD(nArrowmax)),
			COUNTER_FRAME("Rupees", WORD_FIELD(rupies), WORD_FIELD(max_rupees)),
			COUNTER_FRAME("Keys", BYTE_FIELD(keys), WORD_FIELD(max_keys))
		));
		counterTabs[1] = TabRef(name = "Script", Columns<5>(margins = 1_px,
			COUNTER_FRAME("Script 1", WORD_FIELD(scrcnt[0]), WORD_FIELD(scrmaxcnt[0])),
			COUNTER_FRAME("Script 2", WORD_FIELD(scrcnt[1]), WORD_FIELD(scrmaxcnt[1])),
			COUNTER_FRAME("Script 3", WORD_FIELD(scrcnt[2]), WORD_FIELD(scrmaxcnt[2])),
			COUNTER_FRAME("Script 4", WORD_FIELD(scrcnt[3]), WORD_FIELD(scrmaxcnt[3])),
			COUNTER_FRAME("Script 5", WORD_FIELD(scrcnt[4]), WORD_FIELD(scrmaxcnt[4])),
			COUNTER_FRAME("Script 6", WORD_FIELD(scrcnt[5]), WORD_FIELD(scrmaxcnt[5])),
			COUNTER_FRAME("Script 7", WORD_FIELD(scrcnt[6]), WORD_FIELD(scrmaxcnt[6])),
			COUNTER_FRAME("Script 8", WORD_FIELD(scrcnt[7]), WORD_FIELD(scrmaxcnt[7])),
			COUNTER_FRAME("Script 9", WORD_FIELD(scrcnt[8]), WORD_FIELD(scrmaxcnt[8])),
			COUNTER_FRAME("Script 10", WORD_FIELD(scrcnt[9]), WORD_FIELD(scrmaxcnt[9])),
			COUNTER_FRAME("Script 11", WORD_FIELD(scrcnt[10]), WORD_FIELD(scrmaxcnt[10])),
			COUNTER_FRAME("Script 12", WORD_FIELD(scrcnt[11]), WORD_FIELD(scrmaxcnt[11])),
			COUNTER_FRAME("Script 13", WORD_FIELD(scrcnt[12]), WORD_FIELD(scrmaxcnt[12])),
			COUNTER_FRAME("Script 14", WORD_FIELD(scrcnt[13]), WORD_FIELD(scrmaxcnt[13])),
			COUNTER_FRAME("Script 15", WORD_FIELD(scrcnt[14]), WORD_FIELD(scrmaxcnt[14])),
			COUNTER_FRAME("Script 16", WORD_FIELD(scrcnt[15]), WORD_FIELD(scrmaxcnt[15])),
			COUNTER_FRAME("Script 17", WORD_FIELD(scrcnt[16]), WORD_FIELD(scrmaxcnt[16])),
			COUNTER_FRAME("Script 18", WORD_FIELD(scrcnt[17]), WORD_FIELD(scrmaxcnt[17])),
			COUNTER_FRAME("Script 19", WORD_FIELD(scrcnt[18]), WORD_FIELD(scrmaxcnt[18])),
			COUNTER_FRAME("Script 20", WORD_FIELD(scrcnt[19]), WORD_FIELD(scrmaxcnt[19])),
			COUNTER_FRAME("Script 21", WORD_FIELD(scrcnt[20]), WORD_FIELD(scrmaxcnt[20])),
			COUNTER_FRAME("Script 22", WORD_FIELD(scrcnt[21]), WORD_FIELD(scrmaxcnt[21])),
			COUNTER_FRAME("Script 23", WORD_FIELD(scrcnt[22]), WORD_FIELD(scrmaxcnt[22])),
			COUNTER_FRAME("Script 24", WORD_FIELD(scrcnt[23]), WORD_FIELD(scrmaxcnt[23])),
			COUNTER_FRAME("Script 25", WORD_FIELD(scrcnt[24]), WORD_FIELD(scrmaxcnt[24]))
		));
		counterTabs[2] = DummyWidget();
		counterTabs[3] = DummyWidget();
	}
	else
	{
		counterTabs[0] = TabRef(name = "Engine", Rows<2>(
			Rows<2>(
				framed = true, frameText = "Bombs", hAlign = 0.0,
				margins = 2_spx,
				Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
				Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
				WORD_FIELD(nBombs),
				TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
					high = 65535, val = local_zinit.nBombmax,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
					{
						local_zinit.nBombmax = val;
						sBombMax->setVal(SBOMB_RATIO);
					})
			),
			Rows<3>(
				framed = true, frameText = "Super Bombs", hAlign = 0.0,
				margins = 2_spx,
				Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
				Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
				Label(hAlign = 1.0, bottomPadding = 0_px, text = "Ratio"),
				WORD_FIELD(nSbombs),
				sBombMax = TextField(
					maxLength = 5,
					type = GUI::TextField::type::INT_DECIMAL,
					val = SBOMB_RATIO,
					disabled = true
				),
				TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
					high = 255, val = local_zinit.bomb_ratio,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val)
					{
						local_zinit.bomb_ratio = val;
						sBombMax->setVal(SBOMB_RATIO);
					})
			),
			COUNTER_FRAME("Arrows", WORD_FIELD(nArrows), WORD_FIELD(nArrowmax)),
			COUNTER_FRAME("Rupees", WORD_FIELD(rupies), WORD_FIELD(max_rupees)),
			COUNTER_FRAME("Keys", BYTE_FIELD(keys), WORD_FIELD(max_keys))
		));
		counterTabs[1] = TabRef(name = "Scr 1", Columns<3>(margins = 1_px,
			COUNTER_FRAME("Script 1", WORD_FIELD(scrcnt[0]), WORD_FIELD(scrmaxcnt[0])),
			COUNTER_FRAME("Script 2", WORD_FIELD(scrcnt[1]), WORD_FIELD(scrmaxcnt[1])),
			COUNTER_FRAME("Script 3", WORD_FIELD(scrcnt[2]), WORD_FIELD(scrmaxcnt[2])),
			COUNTER_FRAME("Script 4", WORD_FIELD(scrcnt[3]), WORD_FIELD(scrmaxcnt[3])),
			COUNTER_FRAME("Script 5", WORD_FIELD(scrcnt[4]), WORD_FIELD(scrmaxcnt[4])),
			COUNTER_FRAME("Script 6", WORD_FIELD(scrcnt[5]), WORD_FIELD(scrmaxcnt[5])),
			COUNTER_FRAME("Script 7", WORD_FIELD(scrcnt[6]), WORD_FIELD(scrmaxcnt[6])),
			COUNTER_FRAME("Script 8", WORD_FIELD(scrcnt[7]), WORD_FIELD(scrmaxcnt[7])),
			COUNTER_FRAME("Script 9", WORD_FIELD(scrcnt[8]), WORD_FIELD(scrmaxcnt[8]))
		));
		counterTabs[2] = TabRef(name = "Scr 2", Columns<3>(margins = 1_px,
			COUNTER_FRAME("Script 10", WORD_FIELD(scrcnt[9]), WORD_FIELD(scrmaxcnt[9])),
			COUNTER_FRAME("Script 11", WORD_FIELD(scrcnt[10]), WORD_FIELD(scrmaxcnt[10])),
			COUNTER_FRAME("Script 12", WORD_FIELD(scrcnt[11]), WORD_FIELD(scrmaxcnt[11])),
			COUNTER_FRAME("Script 13", WORD_FIELD(scrcnt[12]), WORD_FIELD(scrmaxcnt[12])),
			COUNTER_FRAME("Script 14", WORD_FIELD(scrcnt[13]), WORD_FIELD(scrmaxcnt[13])),
			COUNTER_FRAME("Script 15", WORD_FIELD(scrcnt[14]), WORD_FIELD(scrmaxcnt[14])),
			COUNTER_FRAME("Script 16", WORD_FIELD(scrcnt[15]), WORD_FIELD(scrmaxcnt[15])),
			COUNTER_FRAME("Script 17", WORD_FIELD(scrcnt[16]), WORD_FIELD(scrmaxcnt[16])),
			COUNTER_FRAME("Script 18", WORD_FIELD(scrcnt[17]), WORD_FIELD(scrmaxcnt[17]))
		));
		counterTabs[3] = TabRef(name = "Scr 3", Columns<3>(margins = 1_px,
			COUNTER_FRAME("Script 19", WORD_FIELD(scrcnt[18]), WORD_FIELD(scrmaxcnt[18])),
			COUNTER_FRAME("Script 20", WORD_FIELD(scrcnt[19]), WORD_FIELD(scrmaxcnt[19])),
			COUNTER_FRAME("Script 21", WORD_FIELD(scrcnt[20]), WORD_FIELD(scrmaxcnt[20])),
			COUNTER_FRAME("Script 22", WORD_FIELD(scrcnt[21]), WORD_FIELD(scrmaxcnt[21])),
			COUNTER_FRAME("Script 23", WORD_FIELD(scrcnt[22]), WORD_FIELD(scrmaxcnt[22])),
			COUNTER_FRAME("Script 24", WORD_FIELD(scrcnt[23]), WORD_FIELD(scrmaxcnt[23])),
			COUNTER_FRAME("Script 25", WORD_FIELD(scrcnt[24]), WORD_FIELD(scrmaxcnt[24]))
		));
	}
	
	return Window(
		padding = sized(0_px, 2_spx),
		title = "Init Data",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			padding = sized(0_px, 2_spx),
			TabPanel(
				padding = sized(0_px, 2_spx),
				TabRef(name = "Equipment", Column()),
				TabRef(name = "Counters", TabPanel(
					counterTabs[0],
					counterTabs[1],
					counterTabs[2],
					counterTabs[3]
				)),
				TabRef(name = "LItems", Column()),
				TabRef(name = "Misc", Column()),
				TabRef(name = "Vars", Column())
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					onClick = message::OK),
				Button(
					text = "Cancel",
					onClick = message::CANCEL)
			)
		)
	);
}

bool InitDataDialog::handleMessage(message msg)
{
	switch(msg)
	{
		case message::OK:
		{
			setVals(local_zinit);
		}
		return true;

		case message::CANCEL:
		default:
			return true;
	}
}
