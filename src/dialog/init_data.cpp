#include "init_data.h"
#include "info.h"
#include "gui/use_size.h"
#include <gui/builder.h>
#include "zsys.h"
	
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
	local_zinit(start), setVals(setVals), levelsOffset(0)
{}

void InitDataDialog::setOfs(size_t ofs)
{
	bool _510 = levelsOffset==510;
	levelsOffset = vbound(ofs/10, 0, 51)*10;
	if(!(_510 || levelsOffset==510)) return;
	
	bool vis = levelsOffset!=510;
	for(int q = 2; q < 10; ++q)
	{
		l_lab[q]->setVisible(vis);
		l_maps[q]->setVisible(vis);
		l_comp[q]->setVisible(vis);
		l_bkey[q]->setVisible(vis);
		l_keys[q]->setVisible(vis);
	}
}

//{ Macros
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

#define VAL_FIELD(name, minval, maxval, member) \
Label(text = name, hAlign = 0.0), \
TextField(maxLength = 11, type = GUI::TextField::type::INT_DECIMAL, \
	hAlign = 1.0, low = minval, high = maxval, val = local_zinit.member, \
	width = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
	{ \
		local_zinit.member = val; \
	})

#define DEC_VAL_FIELD(name, minval, maxval, numPlaces, member) \
Label(text = name, hAlign = 0.0), \
TextField(maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL, \
	hAlign = 1.0, low = minval, high = maxval, val = local_zinit.member, \
	width = 4.5_em, places = numPlaces, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
	{ \
		local_zinit.member = val; \
	})

#define COLOR_FIELD(name, member) \
Label(text = name, hAlign = 0.0), \
ColorSel(hAlign = 1.0, val = local_zinit.member, \
	width = 4.5_em, \
	onValChangedFunc = [&](byte val) \
	{ \
		local_zinit.member = val; \
	})

#define LEVEL_FIELD(ind) \
Row( \
	padding = 0_px, \
	l_lab[ind] = Label(text = std::to_string(ind), width = 3_em, textAlign = 2), \
	l_maps[ind] = Checkbox(checked = get_bit(local_zinit.map,ind+levelsOffset)!=0, \
		onToggleFunc = [&](bool state) \
		{ \
			set_bit(local_zinit.map, ind+levelsOffset, state); \
		}), \
	l_comp[ind] = Checkbox(checked = get_bit(local_zinit.compass,ind+levelsOffset)!=0, \
		onToggleFunc = [&](bool state) \
		{ \
			set_bit(local_zinit.compass, ind+levelsOffset, state); \
		}), \
	l_bkey[ind] = Checkbox(checked = get_bit(local_zinit.boss_key,ind+levelsOffset)!=0, \
		onToggleFunc = [&](bool state) \
		{ \
			set_bit(local_zinit.boss_key, ind+levelsOffset, state); \
		}), \
	l_keys[ind] = TextField(maxLength = 3, type = GUI::TextField::type::INT_DECIMAL, \
		val = local_zinit.level_keys[ind+levelsOffset], high = 255, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int val) \
		{ \
			local_zinit.level_keys[ind+levelsOffset] = val; \
		}) \
)

#define BTN_100(val) \
Button(maxwidth = sized(3_em,4_em), padding = 0_px, margins = 0_px, \
	text = ZCGUI_STRINGIZE(val), onClick = message::LEVEL, onPressFunc = [&]() \
	{ \
		setOfs((levelsOffset%100)+val); \
	} \
)

#define BTN_10(val) \
Button(maxwidth = sized(3_em,4_em), padding = 0_px, margins = 0_px, \
	text = ZCGUI_STRINGIZE(val), onClick = message::LEVEL, onPressFunc = [&]() \
	{ \
		setOfs(((levelsOffset/100)*100) + val); \
	} \
)
//}

std::shared_ptr<GUI::Widget> InitDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if(!is_large) //Just return an entirely different dialog...
	{
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
						TabRef(name = "Engine", Rows<2>(
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
						)),
						TabRef(name = "Scr 1", Columns<3>(margins = 1_px,
							COUNTER_FRAME("Script 1", WORD_FIELD(scrcnt[0]), WORD_FIELD(scrmaxcnt[0])),
							COUNTER_FRAME("Script 2", WORD_FIELD(scrcnt[1]), WORD_FIELD(scrmaxcnt[1])),
							COUNTER_FRAME("Script 3", WORD_FIELD(scrcnt[2]), WORD_FIELD(scrmaxcnt[2])),
							COUNTER_FRAME("Script 4", WORD_FIELD(scrcnt[3]), WORD_FIELD(scrmaxcnt[3])),
							COUNTER_FRAME("Script 5", WORD_FIELD(scrcnt[4]), WORD_FIELD(scrmaxcnt[4])),
							COUNTER_FRAME("Script 6", WORD_FIELD(scrcnt[5]), WORD_FIELD(scrmaxcnt[5])),
							COUNTER_FRAME("Script 7", WORD_FIELD(scrcnt[6]), WORD_FIELD(scrmaxcnt[6])),
							COUNTER_FRAME("Script 8", WORD_FIELD(scrcnt[7]), WORD_FIELD(scrmaxcnt[7])),
							COUNTER_FRAME("Script 9", WORD_FIELD(scrcnt[8]), WORD_FIELD(scrmaxcnt[8]))
						)),
						TabRef(name = "Scr 2", Columns<3>(margins = 1_px,
							COUNTER_FRAME("Script 10", WORD_FIELD(scrcnt[9]), WORD_FIELD(scrmaxcnt[9])),
							COUNTER_FRAME("Script 11", WORD_FIELD(scrcnt[10]), WORD_FIELD(scrmaxcnt[10])),
							COUNTER_FRAME("Script 12", WORD_FIELD(scrcnt[11]), WORD_FIELD(scrmaxcnt[11])),
							COUNTER_FRAME("Script 13", WORD_FIELD(scrcnt[12]), WORD_FIELD(scrmaxcnt[12])),
							COUNTER_FRAME("Script 14", WORD_FIELD(scrcnt[13]), WORD_FIELD(scrmaxcnt[13])),
							COUNTER_FRAME("Script 15", WORD_FIELD(scrcnt[14]), WORD_FIELD(scrmaxcnt[14])),
							COUNTER_FRAME("Script 16", WORD_FIELD(scrcnt[15]), WORD_FIELD(scrmaxcnt[15])),
							COUNTER_FRAME("Script 17", WORD_FIELD(scrcnt[16]), WORD_FIELD(scrmaxcnt[16])),
							COUNTER_FRAME("Script 18", WORD_FIELD(scrcnt[17]), WORD_FIELD(scrmaxcnt[17]))
						)),
						TabRef(name = "Scr 3", Columns<3>(margins = 1_px,
							COUNTER_FRAME("Script 19", WORD_FIELD(scrcnt[18]), WORD_FIELD(scrmaxcnt[18])),
							COUNTER_FRAME("Script 20", WORD_FIELD(scrcnt[19]), WORD_FIELD(scrmaxcnt[19])),
							COUNTER_FRAME("Script 21", WORD_FIELD(scrcnt[20]), WORD_FIELD(scrmaxcnt[20])),
							COUNTER_FRAME("Script 22", WORD_FIELD(scrcnt[21]), WORD_FIELD(scrmaxcnt[21])),
							COUNTER_FRAME("Script 23", WORD_FIELD(scrcnt[22]), WORD_FIELD(scrmaxcnt[22])),
							COUNTER_FRAME("Script 24", WORD_FIELD(scrcnt[23]), WORD_FIELD(scrmaxcnt[23])),
							COUNTER_FRAME("Script 25", WORD_FIELD(scrcnt[24]), WORD_FIELD(scrmaxcnt[24]))
						))
					)),
					TabRef(name = "LItems", Column(
						Row(padding = 0_px, margins = 0_px,
							BTN_100(000),
							BTN_100(100),
							BTN_100(200),
							BTN_100(300),
							BTN_100(400),
							BTN_100(500)
						),
						Row(padding = 0_px, margins = 0_px,
							BTN_10(00),
							BTN_10(10),
							BTN_10(20),
							BTN_10(30),
							BTN_10(40),
							BTN_10(50),
							BTN_10(60),
							BTN_10(70),
							BTN_10(80),
							BTN_10(90)
						),
						Columns<6>(
							Row(
								maxheight = 1_em,
								DummyWidget(width = 3_em),
								Label(text = "M", textAlign = 0, width = 9_spx+12_px),
								Label(text = "C", textAlign = 0, width = 9_spx+12_px),
								Label(text = "B", textAlign = 0, width = 9_spx+12_px),
								Label(text = "Key", textAlign = 1, width = 2.5_em)
							),
							LEVEL_FIELD(0),
							LEVEL_FIELD(1),
							LEVEL_FIELD(2),
							LEVEL_FIELD(3),
							LEVEL_FIELD(4),
							Row(
								maxheight = 1_em,
								DummyWidget(width = 3_em),
								Label(text = "M", textAlign = 0, width = 9_spx+12_px),
								Label(text = "C", textAlign = 0, width = 9_spx+12_px),
								Label(text = "B", textAlign = 0, width = 9_spx+12_px),
								Label(text = "Key", textAlign = 1, width = 2.5_em)
							),
							LEVEL_FIELD(5),
							LEVEL_FIELD(6),
							LEVEL_FIELD(7),
							LEVEL_FIELD(8),
							LEVEL_FIELD(9)
						)
					)),
					TabRef(name = "Misc", Column()),
					TabRef(name = "Vars", TabPanel(
						TabRef(name = "1", Row(
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									DEC_VAL_FIELD("Gravity:",1,255,2,gravity),
									DEC_VAL_FIELD("Terminal Vel:",1,9999,2,terminalv),
									VAL_FIELD("Jump Layer Height:",0,255,jump_link_layer_threshold),
									VAL_FIELD("Hero Step:",0,9999,heroStep),
									VAL_FIELD("Subscren Fall Mult:",1,85,subscrSpeed)
								)
							),
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									VAL_FIELD("HP Per Heart:",1,255,hp_per_heart),
									VAL_FIELD("MP Per Block:",1,255,magic_per_block),
									VAL_FIELD("Player Damage Mult:",1,255,hero_damage_multiplier),
									VAL_FIELD("Enemy Damage Mult:",1,255,ene_damage_multiplier),
									COLOR_FIELD("Darkness Color", darkcol)
								)
							)
						)),
						TabRef(name = "2", Row(
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									VAL_FIELD("Light Dither Type:",0,255,dither_type),
									VAL_FIELD("Light Dither Arg:",0,255,dither_arg),
									VAL_FIELD("Light Dither %:",0,255,dither_percent),
									VAL_FIELD("Light Radius:",0,255,def_lightrad),
									VAL_FIELD("Light Transp. %:",0,255,transdark_percent)
								)
							)
						))
					))
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
					TabRef(name = "Engine", Rows<2>(hAlign = 0.0, vAlign = 0.0,
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
					)),
					TabRef(name = "Script", Columns<5>(margins = 1_px,
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
					))
				)),
				TabRef(name = "LItems", Column(
					Row(
						BTN_100(000),
						BTN_100(100),
						BTN_100(200),
						BTN_100(300),
						BTN_100(400),
						BTN_100(500)
					),
					Row(
						BTN_10(00),
						BTN_10(10),
						BTN_10(20),
						BTN_10(30),
						BTN_10(40),
						BTN_10(50),
						BTN_10(60),
						BTN_10(70),
						BTN_10(80),
						BTN_10(90)
					),
					Columns<6>(
						Row(
							maxheight = 1_em,
							DummyWidget(width = 3_em),
							Label(text = "M", textAlign = 0, width = 9_spx+12_px),
							Label(text = "C", textAlign = 0, width = 9_spx+12_px),
							Label(text = "B", textAlign = 0, width = 9_spx+12_px),
							Label(text = "Key", textAlign = 1, width = 2.5_em)
						),
						LEVEL_FIELD(0),
						LEVEL_FIELD(1),
						LEVEL_FIELD(2),
						LEVEL_FIELD(3),
						LEVEL_FIELD(4),
						Row(
							maxheight = 1_em,
							DummyWidget(width = 3_em),
							Label(text = "M", textAlign = 0, width = 9_spx+12_px),
							Label(text = "C", textAlign = 0, width = 9_spx+12_px),
							Label(text = "B", textAlign = 0, width = 9_spx+12_px),
							Label(text = "Key", textAlign = 1, width = 2.5_em)
						),
						LEVEL_FIELD(5),
						LEVEL_FIELD(6),
						LEVEL_FIELD(7),
						LEVEL_FIELD(8),
						LEVEL_FIELD(9)
					)
				)),
				TabRef(name = "Misc", Column()),
				TabRef(name = "Vars", TabPanel(TabRef(name = "", Row(
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							DEC_VAL_FIELD("Gravity:",1,255,2,gravity),
							DEC_VAL_FIELD("Terminal Vel:",1,9999,2,terminalv),
							VAL_FIELD("Jump Layer Height:",0,255,jump_link_layer_threshold),
							VAL_FIELD("Hero Step:",0,9999,heroStep),
							VAL_FIELD("Subscren Fall Mult:",1,85,subscrSpeed),
							VAL_FIELD("HP Per Heart:",1,255,hp_per_heart),
							VAL_FIELD("MP Per Block:",1,255,magic_per_block),
							VAL_FIELD("Player Damage Mult:",1,255,hero_damage_multiplier),
							VAL_FIELD("Enemy Damage Mult:",1,255,ene_damage_multiplier)
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							VAL_FIELD("Light Dither Type:",0,255,dither_type),
							VAL_FIELD("Light Dither Arg:",0,255,dither_arg),
							VAL_FIELD("Light Dither Percentage:",0,255,dither_percent),
							VAL_FIELD("Light Radius:",0,255,def_lightrad),
							VAL_FIELD("Light Transp. Percentage:",0,255,transdark_percent),
							COLOR_FIELD("Darkness Color:", darkcol)
						)
					)
				))))
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
		case message::LEVEL:
		{
			for(int q = 0; q < 10; ++q)
			{
				if(q+levelsOffset > 511)
					break;
				l_lab[q]->setText(std::to_string(q+levelsOffset));
				l_maps[q]->setChecked(get_bit(local_zinit.map,q+levelsOffset)!=0);
				l_comp[q]->setChecked(get_bit(local_zinit.compass,q+levelsOffset)!=0);
				l_bkey[q]->setChecked(get_bit(local_zinit.boss_key,q+levelsOffset)!=0);
				l_keys[q]->setVal(local_zinit.level_keys[q+levelsOffset]);
			}
		}
		return false;
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
