#include "init_data.h"
#include "info.h"
#include "gui/use_size.h"
#include <gui/builder.h>
#include "base/zsys.h"
#include "zc_list_data.h"
#include "zc/ffscript.h"
#include "vectorpick.h"

using std::map;
using std::vector;
extern ListData dmap_list;
extern bool saved;
extern itemdata *itemsbuf;
extern zcmodule moduledata;
extern char *item_string[];
extern script_data *genericscripts[NUMSCRIPTSGENERIC];

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
	local_zinit(start), setVals(setVals), levelsOffset(0), isZC(zc),
	list_dmaps(dmap_list),
	list_items(GUI::ZCListData::itemclass(false)),
	list_genscr(GUI::ZCListData::generic_script())
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
#define SBOMB_RATIO (local_zinit.max_bombs / (local_zinit.bomb_ratio > 0 ? local_zinit.bomb_ratio : 4))

#define BYTE_FIELD(member) \
TextField(maxLength = 3, type = GUI::TextField::type::INT_DECIMAL, \
	high = 255, val = local_zinit.member, \
	fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_zinit.member = val; \
	})

#define VAL_FIELD(t, name, minval, maxval, member, dis) \
Label(text = name, hAlign = 0.0), \
VAL_FIELD_IMPL<t>(minval, maxval, &local_zinit.member, dis)

#define DEC_VAL_FIELD(name, minval, maxval, numPlaces, member, dis) \
Label(text = name, hAlign = 0.0), \
TextField(disabled = dis, maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL, \
	hAlign = 1.0, low = minval, high = maxval, val = local_zinit.member, \
	width = 4.5_em, places = numPlaces, \
	fitParent = true, \
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

//}

std::shared_ptr<GUI::Widget> InitDataDialog::WORD_FIELD(word* member)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
		high = 65535, val = *member,
		fitParent = true,
		onValChangedFunc = [member](GUI::TextField::type,std::string_view,int32_t val)
		{
			*member = val;
		}
	);
}

std::shared_ptr<GUI::Widget> InitDataDialog::COUNTER_FRAME(const char* name, std::shared_ptr<GUI::Widget> field1, std::shared_ptr<GUI::Widget> field2)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Rows<2>(
		framed = true, frameText = name, hAlign = 0.0,
		Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
		Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
		field1,
		field2
	);
}

template <typename T>
std::shared_ptr<GUI::Widget> InitDataDialog::VAL_FIELD_IMPL(T minval, T maxval, T* member, bool dis)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return TextField(disabled = dis, maxLength = 11, type = GUI::TextField::type::INT_DECIMAL,
		hAlign = 1.0, low = minval, high = maxval, val = *member,
		width = 4.5_em,
		fitParent = true,
		onValChangedFunc = [member](GUI::TextField::type,std::string_view,int32_t val)
		{
			*member = val;
		}
	);
}

std::shared_ptr<GUI::Widget> InitDataDialog::LEVEL_FIELD(int ind)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(
		padding = 0_px,
		l_lab[ind] = Label(text = std::to_string(ind), width = 3_em, textAlign = 2),
		l_maps[ind] = Checkbox(checked = get_bit(local_zinit.map,ind+levelsOffset),
			onToggleFunc = [&, ind](bool state)
			{
				set_bit(local_zinit.map, ind+levelsOffset, state);
			}),
		l_comp[ind] = Checkbox(checked = get_bit(local_zinit.compass,ind+levelsOffset),
			onToggleFunc = [&, ind](bool state)
			{
				set_bit(local_zinit.compass, ind+levelsOffset, state);
			}),
		l_bkey[ind] = Checkbox(checked = get_bit(local_zinit.boss_key,ind+levelsOffset),
			onToggleFunc = [&, ind](bool state)
			{
				set_bit(local_zinit.boss_key, ind+levelsOffset, state);
			}),
		l_keys[ind] = TextField(maxLength = 3, type = GUI::TextField::type::INT_DECIMAL,
			val = local_zinit.level_keys[ind+levelsOffset], high = 255,
			onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_zinit.level_keys[ind+levelsOffset] = val;
			})
	);
}

std::shared_ptr<GUI::Widget> InitDataDialog::BTN_100(int val)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	char str[10];
	sprintf(str, "%03d", val);

	return Button(maxwidth = 4_em, padding = 0_px, margins = 0_px,
		text = str, onClick = message::LEVEL, onPressFunc = [&, val]()
		{
			setOfs((levelsOffset%100)+val);
		}
	);
}

std::shared_ptr<GUI::Widget> InitDataDialog::BTN_10(int val)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	char str[10];
	sprintf(str, "%02d", val);

	return Button(maxwidth = 4_em, padding = 0_px, margins = 0_px,
		text = str, onClick = message::LEVEL, onPressFunc = [&, val]()
		{
			setOfs(((levelsOffset/100)*100) + val);
		}
	);
}

std::shared_ptr<GUI::Widget> InitDataDialog::TRICHECK(int ind)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Checkbox(
		checked = get_bit(&(local_zinit.triforce),ind),
		text = std::to_string(ind+1),
		onToggleFunc = [&, ind](bool state)
		{
			set_bit(&local_zinit.triforce,ind,state);
		}
	);
}

static size_t genscr_index = 0;
std::shared_ptr<GUI::Widget> InitDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	// Too many locals error in low-optimization mode for emscripten.
#ifdef EMSCRIPTEN_DEBUG
	return std::shared_ptr<GUI::Widget>(nullptr);
#endif
	
	map<int32_t, map<int32_t, vector<int32_t> > > families;
	icswitcher = Switcher(fitParent = true, hAlign = 0.0, vAlign = 0.0);
	
	for(int32_t q = 0; q < MAXITEMS; ++q)
	{
		int32_t family = itemsbuf[q].family;
		
		if(family == 0x200 || family == itype_triforcepiece || !(itemsbuf[q].flags & ITEM_GAMEDATA))
		{
			continue;
		}
		
        if(families.find(family) == families.end())
        {
            families[family] = map<int32_t, vector<int32_t> >();
        }
		int32_t level = zc_max(1, itemsbuf[q].fam_type);
		
		if(families[family].find(level) == families[family].end())
		{
			families[family][level] = vector<int32_t>();
		}
        
        families[family][level].push_back(q);
	}
	
	int32_t fam_ind = 0;
	for(int32_t q = 0; q < itype_max; ++q)
	{
		auto it = families.find(q);
		if(it == families.end())
		{
			list_items.removeVal(q); //Remove from the lister
			continue;
		}
		switchids[q] = fam_ind++;
		std::shared_ptr<GUI::TabPanel> tbpnl = TabPanel();
		size_t count_in_tab = 0, tabcnt = 0;
		std::shared_ptr<GUI::Grid> grid;
		grid = Columns<15>(fitParent = true,hAlign=0.0,vAlign=0.0);
		for(auto levelit = (*it).second.begin(); levelit != (*it).second.end(); ++levelit)
		{
			for(auto itid = (*levelit).second.begin(); itid != (*levelit).second.end(); ++itid)
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
				if(++count_in_tab >= unsigned(15*2))
				{
					count_in_tab = 0;
					tbpnl->add(
						TabRef(
							name = std::to_string(++tabcnt),
							grid
						));
					grid = Columns<15>(fitParent = true,hAlign=0.0,vAlign=0.0);
				}
			}
		}
		if(count_in_tab)
		{
			tbpnl->add(
				TabRef(
					name = std::to_string(++tabcnt),
					grid
				));
		}
		icswitcher->add(tbpnl);
	}
	
	std::shared_ptr<GUI::Widget> ilist_panel;
	if(switchids.size())
	{
		icswitcher->switchTo(switchids[list_items.getValue(0)]);
		ilist_panel = Row(
			List(minheight = 300_px,
				data = list_items, isABC = true,
				focused = true,
				selectedIndex = 0,
				onSelectFunc = [&](int32_t val)
				{
					icswitcher->switchTo(switchids[val]);
				}
			),
			icswitcher
		);
	}
	else
	{
		icswitcher->add(DummyWidget());
		icswitcher->switchTo(0);
		ilist_panel = Label(text = "No 'Equipment Item's to display!");
	}
	
	std::shared_ptr<GUI::TabPanel> tabs;
	
	window = Window(
		padding = 3_px,
		title = "Init Data",
		onClose = message::CANCEL,
		Column(
			padding = 3_px,
			tabs = TabPanel(
				padding = 3_px,
				TabRef(name = "Equipment", ilist_panel),
				TabRef(name = "Counters", TabPanel(
					TabRef(name = "Engine", Rows<2>(hAlign = 0.0, vAlign = 0.0,
						Rows<2>(
							framed = true, frameText = ZI.getCtrName(crBOMBS), hAlign = 0.0,
							Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
							Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
							WORD_FIELD(&local_zinit.bombs),
							TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
								high = 65535, val = local_zinit.max_bombs,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_zinit.max_bombs = val;
									sBombMax->setVal(SBOMB_RATIO);
								})
						),
						Rows<3>(
							framed = true, frameText = ZI.getCtrName(crSBOMBS), hAlign = 0.0,
							Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
							Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
							Label(hAlign = 1.0, bottomPadding = 0_px, text = "Ratio"),
							WORD_FIELD(&local_zinit.super_bombs),
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
						COUNTER_FRAME(ZI.getCtrName(crARROWS), WORD_FIELD(&local_zinit.arrows), WORD_FIELD(&local_zinit.max_arrows)),
						COUNTER_FRAME(ZI.getCtrName(crMONEY), WORD_FIELD(&local_zinit.rupies), WORD_FIELD(&local_zinit.max_rupees)),
						COUNTER_FRAME(ZI.getCtrName(crKEYS), BYTE_FIELD(keys), WORD_FIELD(&local_zinit.max_keys))
					)),
					TabRef(name = "Custom 1", Columns<5>(margins = 1_px,
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM1), WORD_FIELD(&local_zinit.scrcnt[0]), WORD_FIELD(&local_zinit.scrmaxcnt[0])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM2), WORD_FIELD(&local_zinit.scrcnt[1]), WORD_FIELD(&local_zinit.scrmaxcnt[1])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM3), WORD_FIELD(&local_zinit.scrcnt[2]), WORD_FIELD(&local_zinit.scrmaxcnt[2])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM4), WORD_FIELD(&local_zinit.scrcnt[3]), WORD_FIELD(&local_zinit.scrmaxcnt[3])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM5), WORD_FIELD(&local_zinit.scrcnt[4]), WORD_FIELD(&local_zinit.scrmaxcnt[4])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM6), WORD_FIELD(&local_zinit.scrcnt[5]), WORD_FIELD(&local_zinit.scrmaxcnt[5])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM7), WORD_FIELD(&local_zinit.scrcnt[6]), WORD_FIELD(&local_zinit.scrmaxcnt[6])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM8), WORD_FIELD(&local_zinit.scrcnt[7]), WORD_FIELD(&local_zinit.scrmaxcnt[7])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM9), WORD_FIELD(&local_zinit.scrcnt[8]), WORD_FIELD(&local_zinit.scrmaxcnt[8])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM10), WORD_FIELD(&local_zinit.scrcnt[9]), WORD_FIELD(&local_zinit.scrmaxcnt[9])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM11), WORD_FIELD(&local_zinit.scrcnt[10]), WORD_FIELD(&local_zinit.scrmaxcnt[10])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM12), WORD_FIELD(&local_zinit.scrcnt[11]), WORD_FIELD(&local_zinit.scrmaxcnt[11])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM13), WORD_FIELD(&local_zinit.scrcnt[12]), WORD_FIELD(&local_zinit.scrmaxcnt[12])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM14), WORD_FIELD(&local_zinit.scrcnt[13]), WORD_FIELD(&local_zinit.scrmaxcnt[13])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM15), WORD_FIELD(&local_zinit.scrcnt[14]), WORD_FIELD(&local_zinit.scrmaxcnt[14]))
					)),
					TabRef(name = "Custom 2", Columns<5>(margins = 1_px,
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM16), WORD_FIELD(&local_zinit.scrcnt[15]), WORD_FIELD(&local_zinit.scrmaxcnt[15])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM17), WORD_FIELD(&local_zinit.scrcnt[16]), WORD_FIELD(&local_zinit.scrmaxcnt[16])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM18), WORD_FIELD(&local_zinit.scrcnt[17]), WORD_FIELD(&local_zinit.scrmaxcnt[17])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM19), WORD_FIELD(&local_zinit.scrcnt[18]), WORD_FIELD(&local_zinit.scrmaxcnt[18])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM20), WORD_FIELD(&local_zinit.scrcnt[19]), WORD_FIELD(&local_zinit.scrmaxcnt[19])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM21), WORD_FIELD(&local_zinit.scrcnt[20]), WORD_FIELD(&local_zinit.scrmaxcnt[20])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM22), WORD_FIELD(&local_zinit.scrcnt[21]), WORD_FIELD(&local_zinit.scrmaxcnt[21])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM23), WORD_FIELD(&local_zinit.scrcnt[22]), WORD_FIELD(&local_zinit.scrmaxcnt[22])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM24), WORD_FIELD(&local_zinit.scrcnt[23]), WORD_FIELD(&local_zinit.scrmaxcnt[23])),
						COUNTER_FRAME(ZI.getCtrName(crCUSTOM25), WORD_FIELD(&local_zinit.scrcnt[24]), WORD_FIELD(&local_zinit.scrmaxcnt[24]))
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
							Label(text = "M", textAlign = 0, width = 14_px+12_px),
							Label(text = "C", textAlign = 0, width = 14_px+12_px),
							Label(text = "B", textAlign = 0, width = 14_px+12_px),
							Label(text = "Key", textAlign = 1, width = 2.5_em)
						),
						LEVEL_FIELD(0),
						LEVEL_FIELD(1),
						LEVEL_FIELD(2),
						LEVEL_FIELD(3),
						LEVEL_FIELD(4),
						Row(
							DummyWidget(width = 3_em),
							Label(text = "M", textAlign = 0, width = 14_px+12_px),
							Label(text = "C", textAlign = 0, width = 14_px+12_px),
							Label(text = "B", textAlign = 0, width = 14_px+12_px),
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
					Row(nopad = true,
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
							framed = true, frameText = "Hearts ("+std::string(ZI.getCtrName(crLIFE))+")",
							Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
							Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
							BYTE_FIELD(start_heart),
							BYTE_FIELD(hc)
						),
						Rows<3>(
							framed = true, frameText = ZI.getCtrName(crMAGIC),
							Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
							Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
							Row(padding = 0_px,
								Label(hAlign = 1.0, bottomPadding = 0_px, text = "Drain Rate"),
								INFOBTN_EX("Magic costs are multiplied by this amount. Every time you use a"
									" 'Learn Half Magic' room, this value is halved (rounded down)."
									"\nWhen the 'Show' value on a 'Magic Gauge Piece' subscreen object is"
									" >-1, that piece will only show up when it's 'Show' value is equal to"
									" this value (usable for '1/2', '1/4', '1/8' magic icons; as long as"
									" your starting value is high enough, you can allow stacking several"
									" levels of lowered magic cost)", bottomPadding = 0_px)
							),
							WORD_FIELD(&local_zinit.magic),
							WORD_FIELD(&local_zinit.max_magic),
							BYTE_FIELD(magicdrainrate)
						)
					),
					Columns<2>(
						framed = true, frameText = "Triforce",
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
							VAL_FIELD(byte,"Jump Layer Height:",0,255,jump_hero_layer_threshold,isZC),
							VAL_FIELD(word,"Player Step:",0,9999,heroStep,isZC),
							VAL_FIELD(word,"Subscren Fall Mult:",1,85,subscrSpeed,isZC),
							VAL_FIELD(byte,"HP Per Heart:",1,255,hp_per_heart,false),
							VAL_FIELD(byte,"MP Per Block:",1,255,magic_per_block,false),
							VAL_FIELD(byte,"Player Damage Mult:",1,255,hero_damage_multiplier,false),
							VAL_FIELD(byte,"Enemy Damage Mult:",1,255,ene_damage_multiplier,false)
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							VAL_FIELD(byte,"Light Dither Type:",0,255,dither_type,false),
							VAL_FIELD(byte,"Light Dither Arg:",0,255,dither_arg,false),
							VAL_FIELD(byte,"Light Dither Percentage:",0,255,dither_percent,false),
							VAL_FIELD(byte,"Light Radius:",0,255,def_lightrad,false),
							VAL_FIELD(byte,"Light Transp. Percentage:",0,255,transdark_percent,false),
							COLOR_FIELD("Darkness Color:", darkcol,false),
							VAL_FIELD(int32_t,"Bunny Tile Mod:",-214748,214748,bunny_ltm,false),
							VAL_FIELD(byte,"SwitchHook Style:",0,255,switchhookstyle,false)
						)
					),
					Column(vAlign = 0.0,
						Rows<2>(
							margins = 0_px,
							padding = 0_px,
							DEC_VAL_FIELD("Water Gravity:",-99990000,99990000,4,swimgravity,false),
							VAL_FIELD(word, "Swideswim Up Step:",0,9999,heroSideswimUpStep,false),
							VAL_FIELD(word, "Swideswim Side Step:",0,9999,heroSideswimSideStep,false),
							VAL_FIELD(word, "Swideswim Down Step:",0,9999,heroSideswimDownStep,false),
							DEC_VAL_FIELD("Sideswim Leaving Jump:",-2550000,2550000,4,exitWaterJump,false)
						)
					)
				))))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	if(!isZC)
	{
		tabs->add(TabRef(name = "GenScript", Row(
				List(minheight = 300_px,
					data = list_genscr,
					selectedIndex = genscr_index,
					onSelectFunc = [&](int32_t val)
					{
						genscr_index = (size_t)val;
						genscr_btn->setDisabled(!val);
					}),
				genscr_btn = Button(text = "Edit",
					disabled = !genscr_index,
					onPressFunc = [&]()
					{
						if(genscr_index)
							InitGenscriptWizard(local_zinit,genscr_index).show();
					})
			)));
	}
	
	return window;
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


InitGenscriptWizard::InitGenscriptWizard(zinitdata& start, size_t index):
	local_zinit(start), dest_zinit(start), index(index)
{}
std::shared_ptr<GUI::Widget> InitGenscriptWizard::GEN_INITD(int ind,zasm_meta const& meta)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::string lbl = meta.initd[ind];
	if(lbl.empty())
		lbl = "InitD["+std::to_string(ind)+"]";
	return Row(padding = 0_px,
		Label(text = lbl, hAlign = 1.0),
		Button(forceFitH = true, text = "?",
			hPadding = 0_px,
			disabled = meta.initd_help[ind].empty(),
			onPressFunc = [&, ind]()
			{
				InfoDialog("InitD Info",meta.initd_help[ind]).show();
			}),
		TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = local_zinit.gen_initd[index][ind], swap_type = meta.initd_type[ind],
			onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_zinit.gen_initd[index][ind] = val;
			})
	);
}

#define GEN_EXSTATE_RELOAD         0x01
#define GEN_EXSTATE_CONTINUE       0x02
#define GEN_EXSTATE_CHANGE_SCREEN  0x04
#define GEN_EXSTATE_CHANGE_DMAP    0x08
#define GEN_EXSTATE_CHANGE_LEVEL   0x10

static size_t wiz_tab_pos = 0, wiz_tab_pos2 = 0;
std::shared_ptr<GUI::Widget> InitGenscriptWizard::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	zasm_meta const& meta = genericscripts[index]->meta;
	
	std::shared_ptr<GUI::TabPanel> tabs;
	
	window = Window(
		padding = 3_px,
		title = "Generic Script Data (" + std::to_string(index) + ")",
		onClose = message::CANCEL,
		Column(
			padding = 3_px,
			tabs = TabPanel(ptr = &wiz_tab_pos),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	//Main
	tabs->add(TabRef(name = "Main",
		ScrollingPane(targHeight = 400_px,
			Row(padding = 0_px,
				Rows<2>(vAlign = 0.0,
					Checkbox(
						checked = local_zinit.gen_doscript[index],
						text = "Run from Start",
						onToggleFunc = [&](bool state)
						{
							local_zinit.gen_doscript[index] = state;
						}),
					INFOBTN("Script will run when starting a new save")
				),
				Rows<3>(vAlign = 0.0,
					Row(padding = 0_px,
						Label(text = "Exit States"),
						INFOBTN("Under any of these conditions, the script will STOP running.")
					),
					DummyWidget(),
					Row(padding = 0_px,
						Label(text = "Reload States"),
						INFOBTN("Under any of these conditions,"
							" if the script is currently running, it will be reset to"
							" the start of the script.")
					),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_exitState[index]&GEN_EXSTATE_RELOAD,
						text = "Reload",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_exitState[index],GEN_EXSTATE_RELOAD,state);
						}),
					INFOBTN("When loading from the save select screen"),
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_reloadState[index]&GEN_EXSTATE_RELOAD,
						text = "Reload",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_reloadState[index],GEN_EXSTATE_RELOAD,state);
						}),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_exitState[index]&GEN_EXSTATE_CONTINUE,
						text = "Continue",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_exitState[index],GEN_EXSTATE_CONTINUE,state);
						}),
					INFOBTN("When using the 'Continue' option"),
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_reloadState[index]&GEN_EXSTATE_CONTINUE,
						text = "Continue",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_reloadState[index],GEN_EXSTATE_CONTINUE,state);
						}),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_exitState[index]&GEN_EXSTATE_CHANGE_SCREEN,
						text = "Change Screen",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_exitState[index],GEN_EXSTATE_CHANGE_SCREEN,state);
						}),
					INFOBTN("When changing to a new screen"),
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_reloadState[index]&GEN_EXSTATE_CHANGE_SCREEN,
						text = "Change Screen",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_reloadState[index],GEN_EXSTATE_CHANGE_SCREEN,state);
						}),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_exitState[index]&GEN_EXSTATE_CHANGE_DMAP,
						text = "Change DMap",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_exitState[index],GEN_EXSTATE_CHANGE_DMAP,state);
						}),
					INFOBTN("When changing to a new dmap"),
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_reloadState[index]&GEN_EXSTATE_CHANGE_DMAP,
						text = "Change DMap",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_reloadState[index],GEN_EXSTATE_CHANGE_DMAP,state);
						}),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_exitState[index]&GEN_EXSTATE_CHANGE_LEVEL,
						text = "Change Level",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_exitState[index],GEN_EXSTATE_CHANGE_LEVEL,state);
						}),
					INFOBTN("When changing to a new dmap level"),
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_reloadState[index]&GEN_EXSTATE_CHANGE_LEVEL,
						text = "Change Level",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_reloadState[index],GEN_EXSTATE_CHANGE_LEVEL,state);
						})
				),
				Rows<2>(vAlign = 0.0,
					Label(text = "Event Listens"),
					INFOBTN("If the script calls 'WaitEvent()', these"
						" events will trigger it."),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_INIT),
						text = "Init",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_INIT),state);
						}),
					INFOBTN("When loading the game"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_CONTINUE),
						text = "Continue",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_CONTINUE),state);
						}),
					INFOBTN("When using the 'Continue' option"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_FFC_PRELOAD),
						text = "FFC Preload",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_FFC_PRELOAD),state);
						}),
					INFOBTN("Just before FFC scripts run 'On Screen Init'"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_CHANGE_SCREEN),
						text = "Change Screen",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_CHANGE_SCREEN),state);
						}),
					INFOBTN("When changing to a new screen"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_CHANGE_DMAP),
						text = "Change DMap",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_CHANGE_DMAP),state);
						}),
					INFOBTN("When changing to a new dmap"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_CHANGE_LEVEL),
						text = "Change Level",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_CHANGE_LEVEL),state);
						}),
					INFOBTN("When changing to a new dmap level"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_HERO_HIT_1),
						text = "Hero Hit 1",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_HERO_HIT_1),state);
						}),
					INFOBTN("When the player is hit, before applying ring defense"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_HERO_HIT_2),
						text = "Hero Hit 2",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_HERO_HIT_2),state);
						}),
					INFOBTN("When the player is hit, after applying ring defense"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_COLLECT_ITEM),
						text = "Collect Item",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_COLLECT_ITEM),state);
						}),
					INFOBTN("When an item is collected"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_ENEMY_DROP_ITEM_1),
						text = "Enemy Drop Item 1",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_ENEMY_DROP_ITEM_1),state);
						}),
					INFOBTN("When an enemy is deciding whether or not to drop an item"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_ENEMY_DROP_ITEM_2),
						text = "Enemy Drop Item 2",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_ENEMY_DROP_ITEM_2),state);
						}),
					INFOBTN("When an enemy has dropped an item"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_ENEMY_DEATH),
						text = "Enemy Death",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_ENEMY_DEATH),state);
						}),
					INFOBTN("When an enemy is dying"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_ENEMY_HIT1),
						text = "Enemy Hit 1",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_ENEMY_HIT1),state);
						}),
					INFOBTN("When an enemy is hit, before applying defenses"),
					//
					Checkbox(hAlign = 0.0,
						checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_ENEMY_HIT2),
						text = "Enemy Hit 2",
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_ENEMY_HIT2),state);
						}),
					INFOBTN("When an enemy is hit, after applying defenses")
				),
				Rows<3>(vAlign = 0.0,
					Label(text = "Data Size:"),
					TextField(
						type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
						low = 0, high = 2147480000,
						val = local_zinit.gen_dataSize[index]*10000,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							val /= 10000;
							local_zinit.gen_dataSize[index] = val;
							databtn->setDisabled(!val);
						}),
					INFOBTN("The starting size of the script's 'Data' array."),
					databtn = Button(colSpan = 3, fitParent = true,
						text = "Edit Starting Data",
						disabled = !local_zinit.gen_dataSize[index],
						onPressFunc = [&]()
						{
							if(local_zinit.gen_dataSize[index])
							{
								call_edit_vector(local_zinit.gen_data[index], true, 0,
									local_zinit.gen_dataSize[index]);
							}
						})
				)
			)
		)
	));
	//InitD
	tabs->add(TabRef(name = "InitD",
		Column(padding = 0_px,
			GEN_INITD(0,meta),
			GEN_INITD(1,meta),
			GEN_INITD(2,meta),
			GEN_INITD(3,meta),
			GEN_INITD(4,meta),
			GEN_INITD(5,meta),
			GEN_INITD(6,meta),
			GEN_INITD(7,meta)
		)
	));
	
	return window;
}

bool InitGenscriptWizard::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			dest_zinit.gen_doscript[index] = local_zinit.gen_doscript[index];
			dest_zinit.gen_exitState[index] = local_zinit.gen_exitState[index];
			dest_zinit.gen_reloadState[index] = local_zinit.gen_reloadState[index];
			memcpy(dest_zinit.gen_initd[index], local_zinit.gen_initd[index], sizeof(int32_t)*8);
			dest_zinit.gen_dataSize[index] = local_zinit.gen_dataSize[index];
			dest_zinit.gen_data[index] = local_zinit.gen_data[index];
			dest_zinit.gen_eventstate[index] = local_zinit.gen_eventstate[index];
		}
		return true;

		case message::CANCEL:
		default:
			return true;
	}
}

