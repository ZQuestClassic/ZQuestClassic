#include "init_data.h"
#include "info.h"
#include "gui/use_size.h"
#include <gui/builder.h>
#include "base/zsys.h"
#include "zc_list_data.h"
#include "zc/ffscript.h"
#include "numpick.h"
#include "ditherpick.h"
#include "items.h"
#include "zinfo.h"
#include <fmt/format.h>

using std::map;
using std::vector;

extern bool saved;
extern itemdata *itemsbuf;
extern char *item_string[];
extern script_data *genericscripts[NUMSCRIPTSGENERIC];

static bool life_in_hearts;
void call_geninit_wzrd(zinitdata& start, size_t index)
{
	InitGenscriptWizard(start,index).show();
}
void call_init_dlg(zinitdata& sourcezinit, bool zc)
{
	life_in_hearts = zc_get_config("misc","init_data_life_in_hearts",1);
	bool tmpb = life_in_hearts;
	InitDataDialog(sourcezinit, zc,
		[&sourcezinit](zinitdata const& other)
		{
			saved = false;
			sourcezinit = other;
		}).show();
	if(tmpb != life_in_hearts)
		zc_set_config("misc","init_data_life_in_hearts",life_in_hearts?1:0);
}

InitDataDialog::InitDataDialog(zinitdata const& start, bool zc, std::function<void(zinitdata const&)> setVals):
	local_zinit(start), levelsOffset(0),
	list_dmaps(GUI::ZCListData::dmaps(true)),
	list_items(GUI::ZCListData::itemclass(false)),
	list_genscr(GUI::ZCListData::generic_script()),
	list_bottle_content(GUI::ZCListData::bottletype(true)),
	isZC(zc), setVals(setVals)
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
		l_mcguff[q]->setVisible(vis);
		l_bkill[q]->setVisible(vis);
		l_custom1[q]->setVisible(vis);
		l_custom2[q]->setVisible(vis);
		l_custom3[q]->setVisible(vis);
		l_keys[q]->setVisible(vis);
	}
}

static const GUI::ListData list_region_mapping
{
	{ "Full", REGION_MAPPING_FULL },
	{ "Physical", REGION_MAPPING_PHYSICAL },
};

//{ Macros
#define SBOMB_RATIO (local_zinit.bomb_ratio > 0 ? (local_zinit.mcounter[crBOMBS] / local_zinit.bomb_ratio) : local_zinit.mcounter[crSBOMBS])

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

#define ZFIX_VAL_FIELD(name, minval, maxval, member, dis) \
Label(text = name, hAlign = 0.0), \
TextField(disabled = dis, maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL, \
	hAlign = 1.0, low = minval, high = maxval, val = local_zinit.member.getZLong(), \
	width = 4.5_em, places = 4, \
	fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_zinit.member = zslongToFix(val); \
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

std::shared_ptr<GUI::Widget> InitDataDialog::COUNTER_FRAME(int ctr)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Rows<2>(
		framed = true, frameText = ZI.getCtrName(ctr), hAlign = 0.0,
		Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
		Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
		WORD_FIELD(&local_zinit.counter[ctr]),
		WORD_FIELD(&local_zinit.mcounter[ctr])
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

std::string item_name(int id)
{
	if(unsigned(id) < MAXITEMS)
	{
		return itemsbuf[id].get_name(true);
		//return item_string[id];
	}
	return "";
}

#define CONT_PERC (local_zinit.flags.get(INIT_FL_CONTPERCENT))
#define HP_P_H (local_zinit.hp_per_heart)
#define HEART_FACTOR (life_in_hearts ? HP_P_H : 1)
#define HEART_FACTOR2 ((life_in_hearts && !CONT_PERC) ? HP_P_H : 1)
static size_t genscr_index = 0, maintab = 0, vartab = 0;
std::shared_ptr<GUI::Widget> InitDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	map<int32_t, map<int32_t, vector<int32_t> > > families;
	icswitcher = Switcher(fitParent = true, hAlign = 0.0, vAlign = 0.0);
	
	for(int32_t q = 0; q < MAXITEMS; ++q)
	{
		int32_t family = itemsbuf[q].family;
		
		if(family == 0x200 || family == itype_triforcepiece || !(itemsbuf[q].flags & item_gamedata))
			continue;
		
		if(families.find(family) == families.end())
			families[family] = map<int32_t, vector<int32_t> >();
		
		int32_t level = zc_max(1, itemsbuf[q].fam_type);
		
		if(families[family].find(level) == families[family].end())
			families[family][level] = vector<int32_t>();
		
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
		size_t count_in_tab = 0, tabcnt = 0;
		std::shared_ptr<GUI::Grid> grid = Column(fitParent = true,hAlign=0.0,vAlign=0.0);
		
		for(auto levelit = (*it).second.begin(); levelit != (*it).second.end(); ++levelit)
		{
			for(auto itid = (*levelit).second.begin(); itid != (*levelit).second.end(); ++itid)
			{
				int32_t id = *itid;
				grid->add(Checkbox(
					hAlign=0.0,vAlign=0.0,
					checked = local_zinit.get_item(id),
					text = item_name(id),
					onToggleFunc = [&,id](bool state)
					{
						local_zinit.set_item(id, state);
					}
				));
			}
		}
		icswitcher->add(ScrollingPane(fitParent = true, grid));
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
	
	
	std::shared_ptr<GUI::Grid> scr_ctr_grid = Rows<3>();
	for(int q = crCUSTOM1; q <= crCUSTOM100; ++q)
		scr_ctr_grid->add(COUNTER_FRAME(q));
	std::shared_ptr<GUI::Grid> bottle_content_grid = Rows<2>();
	for(int q = 0; q < NUM_BOTTLE_SLOTS; ++q)
	{
		bottle_content_grid->add(Label(text = fmt::format("Bottle {}:",q), hAlign = 1.0));
		bottle_content_grid->add(DropDownList(
			data = list_bottle_content,
			vPadding = 0_px, minwidth = 200_px,
			selectedValue = local_zinit.bottle_slot[q],
			onSelectFunc = [&, q](int32_t val)
			{
				local_zinit.bottle_slot[q] = val;
			}
		));
	}
	std::shared_ptr<GUI::Widget> counter_panel = TabPanel(
			TabRef(name = "Engine", Column(
				Checkbox(hAlign = 0.0,
					checked = life_in_hearts,
					text = fmt::format("{} in Hearts",ZI.getCtrName(crLIFE)),
					onToggleFunc = [&](bool state)
					{
						life_in_hearts = state;
						refresh_dlg();
					}),
				Rows<3>(hAlign = 0.0, vAlign = 0.0,
					Rows<3>(
						framed = true, frameText = ZI.getCtrName(crLIFE), hAlign = 0.0,
						Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
						Label(hAlign = 0.5, bottomPadding = 0_px, text = "Max"),
						Label(hAlign = 1.0, bottomPadding = 0_px, text = "On Continue"),
						TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
							high = 65535, val = local_zinit.counter[crLIFE] / HEART_FACTOR,
							fitParent = true,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_zinit.counter[crLIFE] = val*HEART_FACTOR;
							}
						),
						TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
							high = 65535, val = local_zinit.mcounter[crLIFE] / HEART_FACTOR,
							fitParent = true,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_zinit.mcounter[crLIFE] = val*HEART_FACTOR;
							}
						),
						TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
							high = 65535, val = local_zinit.cont_heart / HEART_FACTOR2,
							fitParent = true,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_zinit.cont_heart = val*HEART_FACTOR2;
							}
						),
						Button(text = "HP Per Heart", colSpan = 2, maxheight = 1.5_em, fitParent = true,
							onPressFunc = [&](){
								if(auto v = call_get_num("HP Per Heart", HP_P_H, 255, 1))
								{
									auto newhpph = *v;
									if(life_in_hearts)
									{
										local_zinit.counter[crLIFE] = (local_zinit.counter[crLIFE]/HP_P_H) * newhpph;
										local_zinit.mcounter[crLIFE] = (local_zinit.mcounter[crLIFE]/HP_P_H) * newhpph;
										if(!CONT_PERC)
											local_zinit.cont_heart = (local_zinit.cont_heart/HP_P_H) * newhpph;
										refresh_dlg();
									}
									local_zinit.hp_per_heart = newhpph;
								}
							}),
						Checkbox(checked = local_zinit.flags.get(INIT_FL_CONTPERCENT),
							text = "%",
							onToggleFunc = [&](bool state)
							{
								local_zinit.flags.set(INIT_FL_CONTPERCENT,state);
								if(life_in_hearts)
								{
									if(state)
										local_zinit.cont_heart /= HP_P_H;
									else local_zinit.cont_heart *= HP_P_H;
									refresh_dlg();
								}
							}
						)
					),
					Rows<2>(
						framed = true, frameText = ZI.getCtrName(crMAGIC), hAlign = 0.0,
						Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
						Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
						WORD_FIELD(&local_zinit.counter[crMAGIC]),
						WORD_FIELD(&local_zinit.mcounter[crMAGIC]),
						Button(text = "MP Per Block", colSpan = 2, maxheight = 1.5_em, fitParent = true,
							onPressFunc = [&](){
								if(auto v = call_get_num("MP Per Block", local_zinit.magic_per_block, 255, 1))
									local_zinit.magic_per_block = *v;
							})
					),
					COUNTER_FRAME(crMONEY),
					Rows<3>(
						framed = true, frameText = ZI.getCtrName(crSBOMBS), hAlign = 0.0,
						Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
						Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
						Label(hAlign = 1.0, bottomPadding = 0_px, text = "Ratio"),
						WORD_FIELD(&local_zinit.counter[crSBOMBS]),
						sBombMax = TextField(
							maxLength = 5,
							type = GUI::TextField::type::INT_DECIMAL,
							val = SBOMB_RATIO,
							read_only = local_zinit.bomb_ratio
						),
						TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
							high = 255, val = local_zinit.bomb_ratio, disabled = isZC,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_zinit.bomb_ratio = val;
								sBombMax->setReadOnly(val);
								sBombMax->setVal(SBOMB_RATIO);
							})
					),
					Rows<2>(
						framed = true, frameText = ZI.getCtrName(crBOMBS), hAlign = 0.0,
						Label(hAlign = 0.0, bottomPadding = 0_px, text = "Start"),
						Label(hAlign = 1.0, bottomPadding = 0_px, text = "Max"),
						WORD_FIELD(&local_zinit.counter[crBOMBS]),
						TextField(maxLength = 5, type = GUI::TextField::type::INT_DECIMAL,
							high = 65535, val = local_zinit.mcounter[crBOMBS],
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_zinit.mcounter[crBOMBS] = val;
								sBombMax->setVal(SBOMB_RATIO);
							})
					),
					COUNTER_FRAME(crARROWS),
					COUNTER_FRAME(crKEYS)
				)
			)),
			TabRef(name = "Custom", Column(margins = 1_px,
				ScrollingPane(fitParent = true, targHeight = 300_px, scr_ctr_grid)
			)),
			TabRef(name = "Bottles", Column(margins = 1_px,
				ScrollingPane(fitParent = true, targHeight = 300_px, bottle_content_grid)
			))
		);
	
	std::shared_ptr<GUI::TabPanel> tabs;
	
	std::shared_ptr<GUI::Grid> litem_grid = Rows_Columns<10, 6>(spacing = isZC ? 1_px : DEFAULT_PADDING);
	// fill the litems via loop
	for(int ind = 0; ind < 10; ++ind)
	{
		if(ind % 5 == 0) // add header
		{
			litem_grid->add(_d);
			litem_grid->add(Label(text = "M", padding = 0_px));
			litem_grid->add(Label(text = "C", padding = 0_px));
			litem_grid->add(Label(text = "B", padding = 0_px));
			litem_grid->add(Label(text = "T", padding = 0_px));
			litem_grid->add(Label(text = "D", padding = 0_px));
			litem_grid->add(Label(text = "C1", padding = 0_px));
			litem_grid->add(Label(text = "C2", padding = 0_px));
			litem_grid->add(Label(text = "C3", padding = 0_px));
			litem_grid->add(Label(text = "Key", padding = 0_px));
		}
		#define LEVEL_CBOX(arr, flag) \
		arr[ind] = Checkbox(checked = local_zinit.litems[ind+levelsOffset]&flag, \
			padding = 0_px, \
			onToggleFunc = [&, ind](bool state) \
			{ \
				SETFLAG(local_zinit.litems[ind+levelsOffset], flag, state); \
			})
		litem_grid->add(l_lab[ind] = Label(text = std::to_string(ind),
			textAlign = 2, hAlign = 1.0, minwidth = 2_em
		));
		litem_grid->add(LEVEL_CBOX(l_maps, liMAP));
		litem_grid->add(LEVEL_CBOX(l_comp, liCOMPASS));
		litem_grid->add(LEVEL_CBOX(l_bkey, liBOSSKEY));
		litem_grid->add(LEVEL_CBOX(l_mcguff, liTRIFORCE));
		litem_grid->add(LEVEL_CBOX(l_bkill, liBOSS));
		litem_grid->add(LEVEL_CBOX(l_custom1, liCUSTOM01));
		litem_grid->add(LEVEL_CBOX(l_custom2, liCUSTOM02));
		litem_grid->add(LEVEL_CBOX(l_custom3, liCUSTOM03));
		litem_grid->add(l_keys[ind] = TextField(maxLength = 3, type = GUI::TextField::type::INT_DECIMAL,
			val = local_zinit.level_keys[ind+levelsOffset], high = 255, padding = 0_px,
			onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_zinit.level_keys[ind+levelsOffset] = val;
			}
		));
		#undef LEVEL_CBOX
	}
	
	window = Window(
		padding = 3_px,
		title = "Init Data",
		onClose = message::CANCEL,
		Column(
			padding = 3_px,
			tabs = TabPanel(ptr = &maintab,
				padding = 3_px,
				TabRef(name = "Equipment", ilist_panel),
				TabRef(name = "Counters", counter_panel),
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
					Frame(title = "Level Items",
						info = "M = Map"
							"\nC = Compass"
							"\nB = Boss Key"
							"\nT = Dungeon Treasure (McGuffin)"
							"\nD = Boss Defeated"
							"\nC1, C2, C3 = Custom LItems",
						litem_grid
					)
				)),
				TabRef(name = "Vars", TabPanel(ptr = &vartab,
					TabRef(name = "Misc 1", Row(
						Rows<3>(
							Label(text = "Start DMap:"),
							DropDownList(disabled = isZC, data = list_dmaps,
								selectedValue = local_zinit.start_dmap,
								onSelectFunc = [&](int32_t val)
								{
									local_zinit.start_dmap = val;
								}
							),
							INFOBTN("You will spawn on the 'Continue' screen for the specified dmap when starting a new game"),
							Checkbox(
								checked = local_zinit.flags.get(INIT_FL_CANSLASH), fitParent = true,
								colSpan = 2, boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
								text = "Can Sword Slash",
								onToggleFunc = [&](bool state)
								{
									local_zinit.flags.set(INIT_FL_CANSLASH,state);
								}
							),
							INFOBTN("Required to slash with swords. The 'Learn Slash' room type can grant this ability.")
						)
					)),
					TabRef(name = "Misc 2", Row(
						Column(vAlign = 0.0,
							Rows<3>(
								margins = 0_px,
								padding = 0_px,
								VAL_FIELD(byte,"Jump Layer Height:",0,255,jump_hero_layer_threshold,isZC), INFOBTN("Some objects draw higher-layer when their Z is greater than this value"),
								VAL_FIELD(word,"Subscreen Fall Mult:",1,85,subscrSpeed,isZC), INFOBTN("Multiplier of the subscreen's fall speed"),
								VAL_FIELD(byte,"Hero Damage Mult:",1,255,hero_damage_multiplier,false), INFOBTN("This multiplies most damage dealt by the Hero."),
								VAL_FIELD(byte,"Enemy Damage Mult:",1,255,ene_damage_multiplier,false), INFOBTN("This multiplies most damage dealt by enemies."),
								VAL_FIELD(int32_t,"Bunny Tile Mod:",-214748,214748,bunny_ltm,false), INFOBTN("The 'Hero Tile Modifier' added when the Hero is a bunny."),
								//
								VAL_FIELD(byte,"SwitchHook Style:",0,255,switchhookstyle,false),
								INFOBTN("The switch hook effect's default animation style."
									"\n0 = Poof, 1 = Flicker, 2 = Rise/Fall"
									"\nThis applies when a switch effect is triggered by means other than"
									" a SwitchHook item- such as enemy defenses, or SwitchHook Block combo triggers.")
							)
						),
						Column(vAlign = 0.0,
							Rows<3>(
								margins = 0_px,
								padding = 0_px,
								VAL_FIELD(byte, "Flicker Timing:", 0, 255, spriteflickerspeed, false), INFOBTN("How many frames sprites will flicker for. If 0, will hide for the duration of the iframes" + QRHINT({ qr_HEROFLICKER, qr_ENEMIESFLICKER })),
								COLOR_FIELD("Flicker Color:", spriteflickercolor, false), INFOBTN("If not color 0, sprites will flicker to this color. Will not work with 'Old (Faster) Sprite Drawing'" + QRHINT({ qr_HEROFLICKER, qr_ENEMIESFLICKER, qr_OLDSPRITEDRAWS })),
								VAL_FIELD(byte, "Flicker Transparency Passes:", 0, 3, spriteflickertransp, false), INFOBTN("How many transparency passes the flicker effect uses. 0 will be a solid color" + QRHINT({ qr_HEROFLICKER, qr_ENEMIESFLICKER, qr_OLDSPRITEDRAWS })),
								VAL_FIELD(byte, "Heart Pieces:", 0, 255, hcp, false), INFOBTN("Number of Heart Pieces the Hero starts with"),
								VAL_FIELD(byte, "HP Per HC:", 1, 255, hcp_per_hc, false), INFOBTN("Number of Heart Pieces to create a new Heart Container"),
								//
								VAL_FIELD(byte, "Magic Drain Rate:", 0, 255, magicdrainrate, false),
								INFOBTN_EX("Magic costs are multiplied by this amount. Every time you use a"
									" 'Learn Half Magic' room, this value is halved (rounded down)."
									"\nWhen the 'Show' value on a 'Magic Gauge Piece' subscreen object is"
									" >-1, that piece will only show up when its 'Show' value is equal to"
									" this value (usable for '1/2', '1/4', '1/8' magic icons; as long as"
									" your starting value is high enough, you can allow stacking several"
									" levels of lowered magic cost)", bottomPadding = 0_px, forceFitH = true)
									)
								//
						)
					)),
					TabRef(name = "Movement", Row(
						Column(vAlign = 0.0,
							Rows<3>(
								margins = 0_px,
								padding = 0_px,
								DEC_VAL_FIELD("Gravity:", 1, 99990000, 4, gravity, isZC), INFOBTN("The rate of gravity, in px/frame^2"),
								DEC_VAL_FIELD("Terminal Vel:", 1, 999900, 2, terminalv, isZC), INFOBTN("The terminal velocity, in px/frame"),
								ZFIX_VAL_FIELD("Air Drag:", 0, SWAP_MAX, air_drag, isZC), INFOBTN("The 'Air Drag'. This affects how objects with momentum"
									" (such as the Hero, after jumping off of slippery ice) slow down in the air."
									" Measured as a deceleration in px/frame^2"),
								VAL_FIELD(word, "Hero Step:", 0, 65535, heroStep, isZC), INFOBTN("The Hero's movement speed, in 100ths px/frame. Only applies if 'New Hero Movement' is enabled." + QRHINT({ qr_NEW_HERO_MOVEMENT,qr_NEW_HERO_MOVEMENT2 })),
								ZFIX_VAL_FIELD("Hero Shove:", 0, 160000, shove_offset, isZC), INFOBTN("The Hero's 'corner shove' leniency, in pixels. Only applies if 'Newer Hero Movement' is enabled." + QRHINT({ qr_NEW_HERO_MOVEMENT2 }))
							)
						),
						Column(vAlign = 0.0,
							Rows<3>(
								margins = 0_px,
								padding = 0_px,
								DEC_VAL_FIELD("Water Gravity:", -99990000, 99990000, 4, swimgravity, false), INFOBTN("The gravity value used in sideview water"),
								VAL_FIELD(word, "Sideswim Up Step:", 0, 9999, heroSideswimUpStep, false), INFOBTN("The Hero's movement speed in sideview water, upwards"),
								VAL_FIELD(word, "Sideswim Side Step:", 0, 9999, heroSideswimSideStep, false), INFOBTN("The Hero's movement speed in sideview water, sideways"),
								VAL_FIELD(word, "Sideswim Down Step:", 0, 9999, heroSideswimDownStep, false), INFOBTN("The Hero's movement speed in sideview water, downwards"),
								DEC_VAL_FIELD("Sideswim Leaving Jump:", -2550000, 2550000, 4, exitWaterJump, false), INFOBTN("Jump value used when moving out the top of sideview water"),
								VAL_FIELD(byte, "Hero Swim Step Multiplier:", 0, 255, hero_swim_mult, false), INFOBTN("Multiplier applied to movement speed in water, requires 'Newer Hero Movement'" + QRHINT({ qr_NEW_HERO_MOVEMENT2 })),
								VAL_FIELD(byte, "Hero Swim Step Divisor:", 0, 255, hero_swim_div, false), INFOBTN("Divisor applied to movement speed in water, requires 'Newer Hero Movement'" + QRHINT({ qr_NEW_HERO_MOVEMENT2 }))
							)
						)
					)),
					TabRef(name = "Dark Room", Row(
						Column(vAlign = 0.0,
							Rows<3>(
								margins = 0_px,
								padding = 0_px,
								//
								Button(text = "Edit Light Dither Style",
									height = 1.5_em, fitParent = true,
									colSpan = 2,
									onPressFunc = [&]()
									{
										call_edit_dither(local_zinit.dither_type, local_zinit.dither_arg, local_zinit.darkcol, false);
										refresh_dlg();
									}),
								INFOBTN("Determines the design of dither used by dark rooms." + QRHINT({qr_NEW_DARKROOM})),
								//
								VAL_FIELD(byte,"Light Dither Percentage:",0,255,dither_percent,false), INFOBTN("This percentage of each light in dark rooms is added as 'dithered'" + QRHINT({qr_NEW_DARKROOM})),
								VAL_FIELD(byte,"Light Radius:",0,255,def_lightrad,false), INFOBTN("Default light radius, ex. for fire weapons. QR determines if enemy fire is lit." + QRHINT({qr_NEW_DARKROOM, qr_EW_FIRE_EMITS_LIGHT})),
								VAL_FIELD(byte,"Light Transp. Percentage:",0,255,transdark_percent,false), INFOBTN("This percentage of each light in dark rooms is added as 'transparent'" + QRHINT({qr_NEW_DARKROOM})),
								COLOR_FIELD("Darkness Color:", darkcol,false), INFOBTN("The color of darkness" + QRHINT({qr_NEW_DARKROOM})),
								VAL_FIELD(word,"Light Wave Rate:",0,65535,light_wave_rate,false), INFOBTN("The rate at which the light radius 'waves' in and out, in frames." + QRHINT({qr_NEW_DARKROOM})),
								VAL_FIELD(word,"Light Wave Size:",0,65535,light_wave_size,false), INFOBTN("The max size of the light radius 'waves'." + QRHINT({qr_NEW_DARKROOM}))
							)
						)
					)),
					TabRef(name = "Regions", Row(
						Column(vAlign = 0.0,
							Rows<3>(
								Label(text = "Region Mapping:"),
								DropDownList(data = list_region_mapping,
									selectedValue = local_zinit.region_mapping,
									onSelectFunc = [&](int32_t val)
									{
										local_zinit.region_mapping = val;
									}
								),
								INFOBTN("In what way screens should be marked as visited."
									"\nFull: Every screen in a region is mapped upon entry"
									"\nPhysical: Only screens the Hero steps into are mapped")
							)
						)
					))
				))
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
				l_maps[q]->setChecked(local_zinit.litems[q+levelsOffset] & liMAP);
				l_comp[q]->setChecked(local_zinit.litems[q+levelsOffset] & liCOMPASS);
				l_bkey[q]->setChecked(local_zinit.litems[q+levelsOffset] & liBOSSKEY);
				l_mcguff[q]->setChecked(local_zinit.litems[q+levelsOffset] & liTRIFORCE);
				l_bkill[q]->setChecked(local_zinit.litems[q+levelsOffset] & liBOSS);
				l_custom1[q]->setChecked(local_zinit.litems[q+levelsOffset] & liCUSTOM01);
				l_custom2[q]->setChecked(local_zinit.litems[q+levelsOffset] & liCUSTOM02);
				l_custom3[q]->setChecked(local_zinit.litems[q+levelsOffset] & liCUSTOM03);
				l_keys[q]->setVal(local_zinit.level_keys[q+levelsOffset]);
			}
		}
		return false;
		case message::OK:
		{
			local_zinit.cont_heart = std::min(local_zinit.cont_heart, word(CONT_PERC?100:local_zinit.mcounter[crLIFE]));
			local_zinit.hcp = std::min(local_zinit.hcp, byte(local_zinit.hcp_per_hc-1));
			local_zinit.normalize();
			setVals(local_zinit);
		}
		return true;

		case message::CANCEL:
			return true;
	}
	return false;
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
				Rows<3>(vAlign = 0.0,
					Checkbox(
						checked = local_zinit.gen_doscript.get(index),
						text = "Run from Start",
						colSpan = 2,
						onToggleFunc = [&](bool state)
						{
							local_zinit.gen_doscript.set(index, state);
						}),
					INFOBTN("Script will run when starting a new save"),
					//
					Label(text = "Data Size:"),
					TextField(
						type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
						low = 0, high = 214748,
						val = local_zinit.gen_data[index].size(),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							local_zinit.gen_data[index].resize(val);
							databtn->setDisabled(!val);
						}),
					INFOBTN("The starting size of the script's 'Data' array."),
					//
					databtn = Button(colSpan = 3, fitParent = true,
						text = "Edit Starting Data",
						disabled = local_zinit.gen_data[index].empty(),
						onPressFunc = [&]()
						{
							if(!local_zinit.gen_data[index].empty())
								call_edit_map(local_zinit.gen_data[index], true);
						})
				)
			)
		)
	));
	tabs->add(TabRef(name = "Events",
		ScrollingPane(targHeight = 400_px,
			Column(padding = 0_px,
				Row(vAlign = 0.0,
					Label(text = "Event Listens"),
					INFOBTN("If the script calls 'WaitEvent()', these"
						" events will trigger it.")
				),
				Row(vAlign = 0.0,
					Rows<2>(vAlign = 0.0,
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
						INFOBTN("When the Hero is hit, before applying ring defense"),
						//
						Checkbox(hAlign = 0.0,
							checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_HERO_HIT_2),
							text = "Hero Hit 2",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_HERO_HIT_2),state);
							}),
						INFOBTN("When the Hero is hit, after applying ring defense"),
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
						INFOBTN("When an enemy has dropped an item")
					),
					Rows<2>(vAlign = 0.0,
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
						INFOBTN("When an enemy is hit, after applying defenses"),
						//
						Checkbox(hAlign = 0.0,
							checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_POST_COLLECT_ITEM),
							text = "Post Collect Item",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_POST_COLLECT_ITEM),state);
							}),
						INFOBTN("After an item is collected (After the holdup animation completes, if held)"),
						//
						Checkbox(hAlign = 0.0,
							checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_PLAYER_FALL),
							text = "Hero Fall",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_PLAYER_FALL),state);
							}),
						INFOBTN("After the Hero falls in a Pitfall"),
						//
						Checkbox(hAlign = 0.0,
							checked = local_zinit.gen_eventstate[index]&(1<<GENSCR_EVENT_PLAYER_DROWN),
							text = "Hero Drown",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_zinit.gen_eventstate[index],(1<<GENSCR_EVENT_PLAYER_DROWN),state);
							}),
						INFOBTN("After the Hero drowns")
					)
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
			dest_zinit.gen_doscript.set(index, local_zinit.gen_doscript.get(index));
			dest_zinit.gen_exitState[index] = local_zinit.gen_exitState[index];
			dest_zinit.gen_reloadState[index] = local_zinit.gen_reloadState[index];
			dest_zinit.gen_initd[index] = local_zinit.gen_initd[index];
			dest_zinit.gen_data[index] = local_zinit.gen_data[index];
			dest_zinit.gen_eventstate[index] = local_zinit.gen_eventstate[index];
		}
		return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

