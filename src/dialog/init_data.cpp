#include "init_data.h"
#include "info.h"
#include "gui/use_size.h"
#include <gui/builder.h>
#include "util.h"
#include "zsys.h"

extern ListData dmap_list;
extern bool saved;
extern itemdata *itemsbuf;
extern char *item_string[];

void call_init_dlg(zinitdata& sourcezinit, bool zc)
{
    InitDataDialog(sourcezinit, zc,
        [&sourcezinit](zinitdata const& other)
		{
			saved = false;
			sourcezinit = other;
		}).show();
}

InitDataDialog::InitDataDialog(zinitdata const& start, bool zc, std::function<void(zinitdata const&)> setVals):
	local_zinit(start), setVals(setVals), levelsOffset(0), list_dmaps(dmap_list),
	list_items(GUI::ListData::itemclass(false)), isZC(zc)
{}

void InitDataDialog::setOfs(size_t ofs)
{
	bool _510 = levelsOffset==510;
	levelsOffset = vbound(ofs/10, 0, 51)*10;
	if(!(_510 || levelsOffset==510)) return;
	
	bool vis = levelsOffset!=510;
	for(int32_t q = 2; q < 10; ++q)
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
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_zinit.member = val; \
	})

#define WORD_FIELD(member) \
TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL, \
	high = 65535, val = local_zinit.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
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

#define VAL_FIELD(name, minval, maxval, member, dis) \
Label(text = name, hAlign = 0.0), \
TextField(disabled = dis, maxLength = 11, type = GUI::TextField::type::INT_DECIMAL, \
	hAlign = 1.0, low = minval, high = maxval, val = local_zinit.member, \
	width = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_zinit.member = val; \
	})

#define DEC_VAL_FIELD(name, minval, maxval, numPlaces, member, dis) \
Label(text = name, hAlign = 0.0), \
TextField(disabled = dis, maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL, \
	hAlign = 1.0, low = minval, high = maxval, val = local_zinit.member, \
	width = 4.5_em, places = numPlaces, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_zinit.member = val; \
	})

#define COLOR_FIELD(name, member, dis) \
Label(text = name, hAlign = 0.0), \
ColorSel(disabled = dis, hAlign = 1.0, val = local_zinit.member, \
	width = 4.5_em, \
	onValChangedFunc = [&](byte val) \
	{ \
		local_zinit.member = val; \
	})

#define LEVEL_FIELD(ind) \
Row( \
	padding = 0_px, \
	l_lab[ind] = Label(text = std::to_string(ind), width = 3_em, textAlign = 2), \
	l_maps[ind] = Checkbox(checked = get_bit(local_zinit.map,ind+levelsOffset), \
		onToggleFunc = [&](bool state) \
		{ \
			set_bit(local_zinit.map, ind+levelsOffset, state); \
		}), \
	l_comp[ind] = Checkbox(checked = get_bit(local_zinit.compass,ind+levelsOffset), \
		onToggleFunc = [&](bool state) \
		{ \
			set_bit(local_zinit.compass, ind+levelsOffset, state); \
		}), \
	l_bkey[ind] = Checkbox(checked = get_bit(local_zinit.boss_key,ind+levelsOffset), \
		onToggleFunc = [&](bool state) \
		{ \
			set_bit(local_zinit.boss_key, ind+levelsOffset, state); \
		}), \
	l_keys[ind] = TextField(maxLength = 3, type = GUI::TextField::type::INT_DECIMAL, \
		val = local_zinit.level_keys[ind+levelsOffset], high = 255, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
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

#define TRICHECK(ind) \
Checkbox( \
	checked = get_bit(&(local_zinit.triforce),ind), \
	text = std::to_string(ind+1), \
	onToggleFunc = [&](bool state) \
	{ \
		set_bit(&(local_zinit.triforce),ind,state); \
	} \
)
//}

std::shared_ptr<GUI::Widget> InitDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::map<int32_t, std::vector<int32_t>> families;
	icswitcher = Switcher(fitParent = true, hAlign = 0.0, vAlign = 0.0);
	
	for(int32_t q = 0; q < MAXITEMS; ++q)
	{
		int32_t family = itemsbuf[q].family;
		
		if(family == 0x200 || family == itype_triforcepiece || !(itemsbuf[q].flags & ITEM_GAMEDATA))
		{
			continue;
		}
		
		std::map<int32_t,std::vector<int32_t> >::iterator it = families.find(family);
		
        if(it == families.end())
        {
            families[family] = std::vector<int32_t>();
        }
        
        families[family].push_back(q);
	}
	
	int32_t fam_ind = 0;
	for(int32_t q = 0; q < itype_max; ++q)
	{
		std::map<int32_t,std::vector<int32_t> >::iterator it = families.find(q);
		if(it == families.end())
		{
			list_items.removeVal(q); //Remove from the lister
			continue;
		}
		switchids[q] = fam_ind++;
		std::shared_ptr<GUI::Grid> grid = Columns<10>(fitParent = true,hAlign=0.0,vAlign=0.0);
		for(std::vector<int32_t>::iterator itid = (*it).second.begin(); itid != (*it).second.end(); ++itid)
		{
			int32_t id = *itid;
			std::shared_ptr<GUI::Checkbox> cb = Checkbox(
				hAlign=0.0,vAlign=0.0,
				checked = local_zinit.items[id],
				text = item_string[id],
				onToggleFunc = [&,id](bool state)
				{
					local_zinit.items[id] = state;
				}
			);
			grid->add(cb);
		}
		icswitcher->add(grid);
	}
	icswitcher->switchTo(switchids[list_items.getValue(0)]);
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
					TabRef(name = "Equipment", Row(
						List(minheight = 160_px,
							data = list_items, isABC = false,
							selectedIndex = 0,
							onSelectFunc = [&](int32_t val)
							{
								icswitcher->switchTo(switchids[val]);
								broadcast_dialog_message(MSG_DRAW, 0);
							}
						),
						Frame(fitParent = true,
							icswitcher
						)
					)),
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
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
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
									high = 255, val = local_zinit.bomb_ratio, disabled = isZC,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
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
								maxheight = 1.5_em,
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
								maxheight = 1.5_em,
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
					TabRef(name = "Misc", Column(
						Row(
							framed = true,
							Label(text = "Start DMap:"),
							DropDownList(disabled = isZC, data = list_dmaps,
								selectedValue = isZC ? 0 : local_zinit.start_dmap,
								onSelectFunc = [&](int32_t val)
								{
									local_zinit.start_dmap = val;
								}
							)
						),
						Row(
							Row(
								framed = true,
								Label(text = "Continue HP:"),
								BYTE_FIELD(cont_heart),
								Checkbox(checked = get_bit(local_zinit.misc,idM_CONTPERCENT),
									text = "%",
									onToggleFunc = [&](bool state)
									{
										set_bit(local_zinit.misc,idM_CONTPERCENT,state);
									}
								)
							),
							Row(
								framed = true,
								Label(text = "Pieces:"),
								BYTE_FIELD(hcp)
							),
							Row(
								framed = true,
								Label(text = "Per HC:"),
								BYTE_FIELD(hcp_per_hc)
							)
						),
						Row(
							Rows<2>(
								framed = true, frameText = "Hearts",
								Label(hAlign = 0.0, topMargin = 2_px, bottomPadding = 0_px, text = "Start"),
								Label(hAlign = 1.0, topMargin = 2_px, bottomPadding = 0_px, text = "Max"),
								BYTE_FIELD(start_heart),
								BYTE_FIELD(hc)
							),
							Rows<3>(
								framed = true, frameText = "Magic",
								Label(hAlign = 0.0, topMargin = 2_px, bottomPadding = 0_px, text = "Start"),
								Label(hAlign = 1.0, topMargin = 2_px, bottomPadding = 0_px, text = "Max"),
								DummyWidget(),
								WORD_FIELD(magic),
								WORD_FIELD(max_magic),
								Checkbox(
									checked = get_bit(local_zinit.misc,idM_DOUBLEMAGIC),
									text = "Half Cost",
									onToggleFunc = [&](bool state)
									{
										set_bit(local_zinit.misc,idM_DOUBLEMAGIC,state);
									}
								)
							)
						),
						Columns<2>(
							framed = true, frameText = "Triforce",
							topMargin = 2_spx,
							TRICHECK(0),
							TRICHECK(1),
							TRICHECK(2),
							TRICHECK(3),
							TRICHECK(4),
							TRICHECK(5),
							TRICHECK(6),
							TRICHECK(7)
						),
						Row(
							framed = true,
							Checkbox(
								checked = get_bit(local_zinit.misc,idM_CANSLASH),
								text = "Can Slash",
								onToggleFunc = [&](bool state)
								{
									set_bit(local_zinit.misc,idM_CANSLASH,state);
								}
							)
						)
					)),
					TabRef(name = "Vars", TabPanel(
						TabRef(name = "1", Row(
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									DEC_VAL_FIELD("Gravity:",1,99990000,4,gravity2,isZC),
									DEC_VAL_FIELD("Terminal Vel:",1,999900,2,terminalv,isZC),
									VAL_FIELD("Jump Layer Height:",0,255,jump_link_layer_threshold,isZC),
									VAL_FIELD("Hero Step:",0,9999,heroStep,isZC),
									VAL_FIELD("Subscreen Fall Mult:",1,85,subscrSpeed,isZC)
								)
							),
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									VAL_FIELD("HP Per Heart:",1,255,hp_per_heart,false),
									VAL_FIELD("MP Per Block:",1,255,magic_per_block,false),
									VAL_FIELD("Player Damage Mult:",1,255,hero_damage_multiplier,false),
									VAL_FIELD("Enemy Damage Mult:",1,255,ene_damage_multiplier,false),
									COLOR_FIELD("Darkness Color", darkcol,false)
								)
							)
						)),
						TabRef(name = "2", Row(
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									VAL_FIELD("Light Dither Type:",0,255,dither_type,false),
									VAL_FIELD("Light Dither Arg:",0,255,dither_arg,false),
									VAL_FIELD("Light Dither %:",0,255,dither_percent,false),
									VAL_FIELD("Light Radius:",0,255,def_lightrad,false),
									VAL_FIELD("Light Transp. %:",0,255,transdark_percent,false)
								)
							),
							Column(vAlign = 0.0,
								Rows<2>(
									margins = 0_px,
									padding = 0_px,
									DEC_VAL_FIELD("Water Gravity:",-99990000,99990000,4,swimgravity,false),
									VAL_FIELD("Swideswim Up Step:",0,9999,heroSideswimUpStep,false),
									VAL_FIELD("Swideswim Side Step:",0,9999,heroSideswimSideStep,false),
									VAL_FIELD("Swideswim Down Step:",0,9999,heroSideswimDownStep,false),
									DEC_VAL_FIELD("Sideswim Leaving Jump:",-2550000,2550000,4,exitWaterJump,false)
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
				TabRef(name = "Equipment", Row(
					List(minheight = 300_px,
						data = list_items, isABC = false,
						selectedIndex = 0,
						onSelectFunc = [&](int32_t val)
						{
							icswitcher->switchTo(switchids[val]);
							broadcast_dialog_message(MSG_DRAW, 0);
						}
					),
					Frame(fitParent = true,
						icswitcher
					)
				)),
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
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
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
								high = 255, val = local_zinit.bomb_ratio, disabled = isZC,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_zinit.bomb_ratio = val;
									sBombMax->setVal(SBOMB_RATIO);
								})
						),
						COUNTER_FRAME("Arrows", WORD_FIELD(nArrows), WORD_FIELD(nArrowmax)),
						COUNTER_FRAME("Rupees", WORD_FIELD(rupies), WORD_FIELD(max_rupees)),
						COUNTER_FRAME("Keys", BYTE_FIELD(keys), WORD_FIELD(max_keys))
					)),
					TabRef(name = "Script 1", Columns<5>(margins = 1_px,
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
						COUNTER_FRAME("Script 15", WORD_FIELD(scrcnt[14]), WORD_FIELD(scrmaxcnt[14]))
					)),
					TabRef(name = "Script 2", Columns<5>(margins = 1_px,
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
				TabRef(name = "Misc", Column(
					Row(
						framed = true,
						Label(text = "Start DMap:"),
						DropDownList(disabled = isZC, data = list_dmaps,
							selectedValue = local_zinit.start_dmap,
							onSelectFunc = [&](int32_t val)
							{
								local_zinit.start_dmap = val;
							}
						)
					),
					Row(
						Row(
							framed = true,
							Label(text = "Continue HP:"),
							BYTE_FIELD(cont_heart),
							Checkbox(checked = get_bit(local_zinit.misc,idM_CONTPERCENT),
								text = "%",
								onToggleFunc = [&](bool state)
								{
									set_bit(local_zinit.misc,idM_CONTPERCENT,state);
								}
							)
						),
						Row(
							framed = true,
							Label(text = "Pieces:"),
							BYTE_FIELD(hcp)
						),
						Row(
							framed = true,
							Label(text = "Per HC:"),
							BYTE_FIELD(hcp_per_hc)
						)
					),
					Row(
						Rows<2>(
							framed = true, frameText = "Hearts",
							Label(hAlign = 0.0, topMargin = 2_px, bottomPadding = 0_px, text = "Start"),
							Label(hAlign = 1.0, topMargin = 2_px, bottomPadding = 0_px, text = "Max"),
							BYTE_FIELD(start_heart),
							BYTE_FIELD(hc)
						),
						Rows<3>(
							framed = true, frameText = "Magic",
							Label(hAlign = 0.0, topMargin = 2_px, bottomPadding = 0_px, text = "Start"),
							Label(hAlign = 1.0, topMargin = 2_px, bottomPadding = 0_px, text = "Max"),
							DummyWidget(),
							WORD_FIELD(magic),
							WORD_FIELD(max_magic),
							Checkbox(
								checked = get_bit(local_zinit.misc,idM_DOUBLEMAGIC),
								text = "Half Cost",
								onToggleFunc = [&](bool state)
								{
									set_bit(local_zinit.misc,idM_DOUBLEMAGIC,state);
								}
							)
						)
					),
					Columns<2>(
						framed = true, frameText = "Triforce",
						topMargin = 2_spx,
						TRICHECK(0),
						TRICHECK(1),
						TRICHECK(2),
						TRICHECK(3),
						TRICHECK(4),
						TRICHECK(5),
						TRICHECK(6),
						TRICHECK(7)
					),
					Row(
						framed = true,
						Checkbox(
							checked = get_bit(local_zinit.misc,idM_CANSLASH),
							text = "Can Slash",
							onToggleFunc = [&](bool state)
							{
								set_bit(local_zinit.misc,idM_CANSLASH,state);
							}
						)
					)
				)),
				TabRef(name = "Vars", TabPanel(TabRef(name = "", Row(
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							DEC_VAL_FIELD("Gravity:",1,99990000,4,gravity2,isZC),
							DEC_VAL_FIELD("Terminal Vel:",1,999900,2,terminalv,isZC),
							VAL_FIELD("Jump Layer Height:",0,255,jump_link_layer_threshold,isZC),
							VAL_FIELD("Hero Step:",0,9999,heroStep,isZC),
							VAL_FIELD("Subscren Fall Mult:",1,85,subscrSpeed,isZC),
							VAL_FIELD("HP Per Heart:",1,255,hp_per_heart,false),
							VAL_FIELD("MP Per Block:",1,255,magic_per_block,false),
							VAL_FIELD("Player Damage Mult:",1,255,hero_damage_multiplier,false),
							VAL_FIELD("Enemy Damage Mult:",1,255,ene_damage_multiplier,false)
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							VAL_FIELD("Light Dither Type:",0,255,dither_type,false),
							VAL_FIELD("Light Dither Arg:",0,255,dither_arg,false),
							VAL_FIELD("Light Dither Percentage:",0,255,dither_percent,false),
							VAL_FIELD("Light Radius:",0,255,def_lightrad,false),
							VAL_FIELD("Light Transp. Percentage:",0,255,transdark_percent,false),
							COLOR_FIELD("Darkness Color:", darkcol,false)
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							DEC_VAL_FIELD("Water Gravity:",-99990000,99990000,4,swimgravity,false),
							VAL_FIELD("Swideswim Up Step:",0,9999,heroSideswimUpStep,false),
							VAL_FIELD("Swideswim Side Step:",0,9999,heroSideswimSideStep,false),
							VAL_FIELD("Swideswim Down Step:",0,9999,heroSideswimDownStep,false),
							DEC_VAL_FIELD("Sideswim Leaving Jump:",-2550000,2550000,4,exitWaterJump,false)
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

bool InitDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::LEVEL:
		{
			for(int32_t q = 0; q < 10; ++q)
			{
				if(q+levelsOffset > 511)
					break;
				l_lab[q]->setText(std::to_string(q+levelsOffset));
				l_maps[q]->setChecked(get_bit(local_zinit.map,q+levelsOffset));
				l_comp[q]->setChecked(get_bit(local_zinit.compass,q+levelsOffset));
				l_bkey[q]->setChecked(get_bit(local_zinit.boss_key,q+levelsOffset));
				l_keys[q]->setVal(local_zinit.level_keys[q+levelsOffset]);
			}
		}
		return false;
		case message::OK:
		{
			local_zinit.cont_heart = std::min(local_zinit.cont_heart, word(get_bit(local_zinit.misc,idM_CONTPERCENT)?100:local_zinit.hc));
			local_zinit.hcp = std::min(local_zinit.hcp, byte(local_zinit.hcp_per_hc-1));
			setVals(local_zinit);
		}
		return true;

		case message::CANCEL:
		default:
			return true;
	}
}
