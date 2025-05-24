#include "subscr_props.h"
#include <gui/builder.h>
#include <gui/buildutil.h>
#include "info.h"
#include <utility>
#include <sstream>
#include <fmt/format.h>
#include "sprite.h"
#include "zq/zq_subscr.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"
#include "base/misctypes.h"
#include "subscr_transition.h"
#include "subscr_macros.h"
#include "items.h"

extern script_data *genericscripts[NUMSCRIPTSGENERIC];
extern ZCSubscreen subscr_edit;
extern itemdata *itemsbuf;

static bool dlg_retval = false;
bool call_subscrprop_dialog(SubscrWidget* widg, int32_t obj_ind)
{
	SubscrPropDialog(widg,obj_ind).show();
	return dlg_retval;
}

static const int btn_flags[4] = {INT_BTN_A,INT_BTN_B,INT_BTN_X,INT_BTN_Y};
SubscrPropDialog::SubscrPropDialog(SubscrWidget* widg, int32_t obj_ind) :
	subref(widg), local_subref(widg->clone()), index(obj_ind),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_shadtype(GUI::ZCListData::shadow_types()),
	list_aligns(GUI::ZCListData::alignments()),
	list_buttons(GUI::ZCListData::buttons()),
	list_items(GUI::ZCListData::items(true)),
	list_items_no_none(GUI::ZCListData::items(true, false)),
	list_counters(GUI::ZCListData::ss_counters(true)), //All counters
	list_counters2(GUI::ZCListData::ss_counters(true,true)), //All counters, no (None)
	list_itemclass(GUI::ZCListData::itemclass(true)),
	list_genscr(GUI::ZCListData::generic_script()),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_costinds(GUI::ListData::numbers(false, 0, 2))
{
	byte pg = subscr_edit.curpage, ind = index;
	start_default_btnslot = 0;
	if(widg->getType() == widgITEMSLOT)
	{
		for(int q = 0; q < 4; ++q)
		{
			if((subscr_edit.def_btns[q]&0xFF) == pg
				&& (subscr_edit.def_btns[q]>>8) == subref->pos)
				start_default_btnslot |= btn_flags[q];
		}
	}
	set_default_btnslot = start_default_btnslot;
}

static const GUI::ListData two_three_rows
{
	{ "Two", 2 },
	{ "Three", 3 }
};
static const GUI::ListData special_tile_list
{
	{ "None", -1 },
	{ "SS Vine", 0 },
	{ "Magic Meter", 1 }
};
static const GUI::ListData wrapping_type_list
{
	{ "Character", 0 },
	{ "Word", 1 }
};
static const GUI::ListData list_pgmode
{
	{ "None", PGGOTO_NONE },
	{ "Next", PGGOTO_NEXT },
	{ "Prev", PGGOTO_PREV },
	{ "Target", PGGOTO_TRG },
};
static const GUI::ListData list_condty
{
	{ "(None)", CONDTY_NONE },
	{ "==", CONDTY_EQ },
	{ "!=", CONDTY_NEQ },
	{ ">", CONDTY_GREATER },
	{ ">=", CONDTY_GREATEREQ },
	{ "<", CONDTY_LESS },
	{ "<=", CONDTY_LESSEQ },
};

#define MISC_COLOR_SEL(var, txt, num) \
Frame( \
	title = txt, \
	col_sel[num-1] = MiscColorSel( \
		c1 = var.type, \
		c2 = var.color, \
		onUpdate = [=](int32_t c1, int32_t c2) \
		{ \
			var.type = c1; \
			var.color = c2; \
			updateColors(); \
		}) \
)
#define MISC_COLOR_SEL_EX(var, txt, num, ...) \
Frame( \
	__VA_ARGS__, \
	title = txt, \
	col_sel[num-1] = MiscColorSel( \
		c1 = var.type, \
		c2 = var.color, \
		onUpdate = [=](int32_t c1, int32_t c2) \
		{ \
			var.type = c1; \
			var.color = c2; \
			updateColors(); \
		}) \
)

#define MISC_CSET_SEL(var, txt, num) \
Frame( \
	title = txt, \
	cs_sel[num-1] = MiscCSetSel( \
		c1 = var.type, \
		c2 = var.color, \
		onUpdate = [=](int32_t c1, int32_t c2) \
		{ \
			var.type = c1; \
			var.color = c2; \
			updateColors(); \
		}) \
)

#define DDL_FONT(var) \
DropDownList(data = list_font, \
	fitParent = true, \
	selectedValue = var, \
	onSelectFunc = [=](int32_t val) \
	{ \
		var = val; \
		if(fonttf) fonttf->setFont(get_zc_font(val)); \
	} \
)

#define GAUGE_MINITILE(ind,txt,vMTInfo,vModflag,bit) \
Frame(fitParent = true, Column(fitParent = true, \
	Label(/*useFont = spfont, */text = txt), \
	gauge_tswatches[ind] = SelTileSwatch( \
		hAlign = 0.0, \
		tile = vMTInfo.tile(), \
		cset = vMTInfo.cset, \
		mini = true, \
		minicorner = vMTInfo.crn(), \
		showvals = false, \
		showT0 = true, \
		onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t crn) \
		{ \
			vMTInfo.setTileCrn(t,crn); \
			vMTInfo.cset = c; \
		} \
	), \
	CBOX(vModflag,bit,"Mod",1) \
))

#define DEFEQUP_CBOX(ind,btnstr,qrhint) \
def_eqp_cboxes[ind] = Checkbox( \
	text = fmt::format("Default {} Equipment",btnstr), hAlign = 0.0, \
	checked = set_default_btnslot&btn_flags[ind], \
	onToggleFunc = [=](bool state) \
	{ \
		if(state) \
			set_default_btnslot = btn_flags[ind]; \
		else set_default_btnslot &= ~btn_flags[ind]; \
		for(int cb = 0; cb < 4; ++cb) \
		{ \
			if(cb==ind) continue; \
			def_eqp_cboxes[cb]->setChecked(false); \
		} \
	}), \
INFOBTN_F("Sets this ItemSlot to be the default equipped to the '{0}' button." \
	" This applies only for the active subscreen on the Starting DMap." \
	" Setting this will unset it for all other ItemSlots." \
	"\nThis can only be set if items can be equipped to the {0} button," \
	" and 'qr_OLD_SUBSCR' is disabled.{1}",btnstr,qrhint)

//Tile block max preview tiledim
#define TB_LA 12

char* repl_escchar(char* buf, char const* ptr, bool compact)
{
	size_t len = strlen(ptr);
	size_t pos = 0;
	if(compact) // "\\n" -> '\n', etc
	{
		if(!buf) buf = new char[len+1];
		for(size_t q = 0; q < len; ++q)
		{
			if(q+1 < len)
			{
				if(ptr[q] == '\\')
				{
					switch(ptr[q+1])
					{
						case 't':
							buf[pos++] = '\t';
							++q;
							continue;
						case 'n':
							buf[pos++] = '\n';
							++q;
							continue;
						case '\\':
							buf[pos++] = '\\';
							++q;
							continue;
					}
				}
				buf[pos++] = ptr[q];
			}
		}
		return buf;
	}
	else // '\n' -> "\\n", etc
	{
		std::ostringstream tmp;
		for(size_t q = 0; q < len; ++q)
		{
			switch(ptr[q])
			{
				case '\n':
					tmp << "\\n";
					break;
				case '\t':
					tmp << "\\t";
					break;
				case '\\':
					tmp << "\\\\";
					break;
				default:
					tmp << ptr[q];
					break;
			}
		}
		std::string str = tmp.str();
		if(!buf) buf = new char[str.size()+1];
		strcpy(buf, str.c_str());
		return buf;
	}
}

#define INITD_LAB_WIDTH 12_em
std::shared_ptr<GUI::Widget> SubscrPropDialog::GEN_INITD(int ind)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::string lbl = local_gen_meta.initd[ind];
	if(lbl.empty())
		lbl = "InitD["+std::to_string(ind)+"]:";
	return Row(padding = 0_px, hAlign = 1.0,
		geninitd_lbl[ind] = Label(minwidth = INITD_LAB_WIDTH, text = lbl, textAlign = 2),
		geninitd_btn[ind] = Button(forceFitH = true, text = "?",
			hPadding = 0_px,
			disabled = local_gen_meta.initd_help[ind].empty(),
			onPressFunc = [&, ind]()
			{
				InfoDialog("InitD Info",local_gen_meta.initd_help[ind]).show();
			}),
		TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = local_subref->generic_initd[ind], swap_type = local_gen_meta.initd_type[ind],
			onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_subref->generic_initd[ind] = val;
			})
	);
}

enum
{
	CI_REQ,
	CI_PICKED,
	CI_REQ_NOT,
	NUM_CI
};

static size_t sprop_tabs[widgMAX] = {0};
static size_t sprop_tab_sel = 0;
static char tbuf[1025] = {0};
static int cond_item_sels[NUM_CI] = {0};
std::shared_ptr<GUI::Widget> SubscrPropDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::string titlebuf = fmt::format("{} Properties (Object #{})",
		local_subref->getTypeName(), index);
	
	std::shared_ptr<GUI::Grid> loc_grid;
	//Generate 'location' grid
	{
		//Generate any type-specific location data (ex special w/h things)
		word loadw = local_subref->getW();
		word loadh = local_subref->getH();
		bool show_xy = true;
		bool show_wh = true;
		switch(local_subref->getType())
		{
			case widgFRAME:
			case widgITEMSLOT:
			case widgMCGUFF:
			case widgTILEBLOCK:
			case widgTEXTBOX:
				loadw = local_subref->w;
				loadh = local_subref->h;
				break;
			case widgTEXT:
			case widgMMETER:
			case widgLMETER:
			case widgMMAP:
			case widgLMAP:
			case widgSELECTOR:
			case widgMINITILE:
			case widgMCGUFF_FRAME:
			case widgLGAUGE:
			case widgMGAUGE:
			case widgCOUNTER:
			case widgOLDCTR:
			case widgBTNCOUNTER:
				show_wh = false;
				break;
			case widgBGCOLOR:
				show_xy = false;
				show_wh = false;
				break;
			case widgMMAPTITLE:
				if (get_qr(qr_OLD_DMAP_INTRO_STRINGS))
					show_wh = false;
				else
				{
					loadw = local_subref->w;
					loadh = local_subref->h;
				}
				break;
			default: break;
		}
		std::shared_ptr<GUI::Grid> g1;
		loc_grid = Column(
				Row(
					Rows<2>(
						Label(text = "Display:", colSpan = 2),
						INFOBTN("Draws while the active subscreen is UP (closed)"),
						Checkbox(
							text = "Active Up", hAlign = 0.0,
							checked = local_subref->posflags & sspUP,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspUP,state);
							}
						),
						INFOBTN("Draws while the active subscreen is DOWN (open)"),
						Checkbox(
							text = "Active Down", hAlign = 0.0,
							checked = local_subref->posflags & sspDOWN,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspDOWN,state);
							}
						),
						INFOBTN("Draws while the active subscreen is SCROLLING (opening/closing)"),
						Checkbox(
							text = "Active Scrolling", hAlign = 0.0,
							checked = local_subref->posflags & sspSCROLLING,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspSCROLLING,state);
							}
						),
						INFOBTN("Hide if a message string is showing."),
						Checkbox(
							text = "Hide for String", hAlign = 0.0,
							checked = local_subref->posflags & sspNOMSGSTR,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspNOMSGSTR,state);
							}
						)
					),
					g1 = Rows<2>()
				)
			);
		if(show_xy)
		{
			g1->add(Label(text = "X:"));
			g1->add(NUM_FIELD(local_subref->x,-999,9999));
			g1->add(Label(text = "Y:"));
			g1->add(NUM_FIELD(local_subref->y,-999,9999));
		}
		if(show_wh)
		{
			g1->add(Label(text = "W:"));
			g1->add(TextField(
				fitParent = true,
				type = GUI::TextField::type::INT_DECIMAL,
				low = 0, high = 999, val = loadw,
				onValChangedFunc = [=](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_subref->w = val;
					update_wh();
				}));
			g1->add(Label(text = "H:"));
			g1->add(TextField(
				fitParent = true,
				type = GUI::TextField::type::INT_DECIMAL,
				low = 0, high = 999, val = loadh,
				onValChangedFunc = [=](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_subref->h = val;
					update_wh();
				}));
		}
		else if(!show_xy)
		{
			g1->add(DummyWidget());
		}
		
	}
	
	std::shared_ptr<GUI::Grid> col_grid;
	//Generate 'color' grid
	{
		switch(local_subref->getType())
		{
			case widgFRAME:
			{
				SW_2x2Frame* w = dynamic_cast<SW_2x2Frame*>(local_subref);
				col_grid = Column(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgTIME:
			{
				SW_Time* w = dynamic_cast<SW_Time*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgBTNITM:
				break;
			case widgCOUNTER:
			{
				SW_Counter* w = dynamic_cast<SW_Counter*>(local_subref);
				col_grid = Columns<3>(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3),
					MISC_COLOR_SEL_EX(w->c_text2, "'Full' Text Color", 4, info = "Color used when counter is full"),
					MISC_COLOR_SEL_EX(w->c_shadow2, "'Full' Shadow Color", 5, info = "Color used when counter is full"),
					MISC_COLOR_SEL_EX(w->c_bg2, "'Full' Background Color", 6, info = "Color used when counter is full"));
				break;
			}
			case widgBTNCOUNTER:
			{
				SW_BtnCounter* w = dynamic_cast<SW_BtnCounter*>(local_subref);
				col_grid = Columns<3>(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3),
					MISC_COLOR_SEL_EX(w->c_text2, "'Full' Text Color", 4, info = "Color used when counter is full"),
					MISC_COLOR_SEL_EX(w->c_shadow2, "'Full' Shadow Color", 5, info = "Color used when counter is full"),
					MISC_COLOR_SEL_EX(w->c_bg2, "'Full' Background Color", 6, info = "Color used when counter is full"));
				break;
			}
			case widgOLDCTR:
			{
				SW_Counters* w = dynamic_cast<SW_Counters*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgITEMSLOT:
				break;
			case widgBGCOLOR:
			{
				SW_Clear* w = dynamic_cast<SW_Clear*>(local_subref);
				col_grid = Column(MISC_COLOR_SEL(w->c_bg, "Subscreen Color", 1));
				break;
			}
			case widgLMAP:
			{
				SW_LMap* w = dynamic_cast<SW_LMap*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_room, "Room Color", 1),
					MISC_COLOR_SEL(w->c_plr, "Hero Color", 2));
				break;
			}
			case widgLGAUGE:
				break;
			case widgLMETER:
				break;
			case widgLINE:
			{
				SW_Line* w = dynamic_cast<SW_Line*>(local_subref);
				col_grid = Column(MISC_COLOR_SEL(w->c_line, "Line Color", 1));
				break;
			}
			case widgMGAUGE:
				break;
			case widgMMETER:
				break;
			case widgMMAP:
			{
				SW_MMap* w = dynamic_cast<SW_MMap*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL_EX(w->c_plr, "Hero Color", 1, info = "The color of the 'you are here' position"),
					MISC_COLOR_SEL_EX(w->c_cmp_blink, "Compass Blink Color", 2, info = "The color the compass marker blinks to, when active"),
					MISC_COLOR_SEL_EX(w->c_cmp_off, "Compass Const Color", 3, info = "The color the compass marker stays when inactive, and blinks from while active"));
				break;
			}
			case widgMMAPTITLE:
			{
				SW_MMapTitle* w = dynamic_cast<SW_MMapTitle*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgMINITILE:
			{
				SW_MiniTile* w = dynamic_cast<SW_MiniTile*>(local_subref);
				col_grid = Column(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgRECT:
			{
				SW_Rect* w = dynamic_cast<SW_Rect*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_outline, "Outline Color", 1),
					MISC_COLOR_SEL(w->c_fill, "Fill Color", 2));
				break;
			}
			case widgSELECTEDTEXT:
			{
				SW_SelectedText* w = dynamic_cast<SW_SelectedText*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgSELECTOR:
				break;
			case widgTEXT:
			{
				SW_Text* w = dynamic_cast<SW_Text*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgTEXTBOX:
			{
				SW_TextBox* w = dynamic_cast<SW_TextBox*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_text, "Text Color", 1),
					MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2),
					MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgTILEBLOCK:
			{
				SW_TileBlock* w = dynamic_cast<SW_TileBlock*>(local_subref);
				col_grid = Column(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgMCGUFF_FRAME:
			{
				SW_TriFrame* w = dynamic_cast<SW_TriFrame*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_outline, "Frame Outline Color", 1),
					MISC_COLOR_SEL(w->c_number, "Number Color", 2));
				break;
			}
			case widgMCGUFF:
			{
				SW_McGuffin* w = dynamic_cast<SW_McGuffin*>(local_subref);
				col_grid = Column(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgCOUNTERPERCBAR:
			{
				SW_CounterPercentBar* w = dynamic_cast<SW_CounterPercentBar*>(local_subref);
				col_grid = Column(
					MISC_COLOR_SEL(w->c_fill, "Fill Color", 1),
					MISC_COLOR_SEL(w->c_bg, "BG Color", 2));
				break;
			}
		}
	}
	
	std::shared_ptr<GUI::Grid> attrib_grid;
	TabBuilder extra_grids;
	enum { mtNONE, mtFORCE_TAB, mtLOCTOP };
	int32_t mergetype = mtNONE;
	//Generate 'attributes' grid
	{
		auto widgty = local_subref->getType();
		switch(widgty)
		{
			case widgFRAME:
			{
				SW_2x2Frame* w = dynamic_cast<SW_2x2Frame*>(local_subref);
				attrib_grid = Column(
					tswatches[0] = SelTileSwatch(
						hAlign = 0.0,
						tile = w->tile,
						cset = w->cs.get_cset(),
						//flip = local_subref->d2,
						tilewid = 2, tilehei = 2,
						showvals = false,
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t)
						{
							w->tile = t;
							if(w->cs.type != ssctMISC)
							{
								w->cs.type = c;
								cs_sel[0]->setC1(c);
							}
						}
					),
					CBOX(w->flags,SUBSCR_2X2FR_OVERLAY,"Overlay",1),
					CBOX(w->flags,SUBSCR_2X2FR_TRANSP,"Transparent",1)
				);
				break;
			}
			case widgTIME:
			{
				SW_Time* w = dynamic_cast<SW_Time*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Columns<4>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					INFOBTN("Changes the format the time displays in from 'H:MM:SS' to 'HH;MM'"
						" (where the ';' flickers back and forth between ':' and ';' twice/second)"),
					DDL_FONT(w->fontid),
					DDL(w->shadtype, list_shadtype),
					DDL(w->align, list_aligns),
					CBOX(w->flags,SUBSCR_TIME_ALTSTR,"Alt Format",1)
				);
				break;
			}
			case widgBTNITM:
			{
				SW_ButtonItem* w = dynamic_cast<SW_ButtonItem*>(local_subref);
				attrib_grid = Rows<2>(
					Label(text = "Button:", hAlign = 1.0),
					DDL(w->btn, list_buttons),
					CBOX(w->flags,SUBSCR_BTNITM_TRANSP,"Transparent",2)
				);
				break;
			}
			case widgCOUNTER:
			{
				SW_Counter* w = dynamic_cast<SW_Counter*>(local_subref);
				mergetype = mtFORCE_TAB; //too wide to fit!
				attrib_grid = Row(
					Rows<2>(
						Label(text = "Font:", hAlign = 1.0),
						DDL_FONT(w->fontid),
						Label(text = "Style:", hAlign = 1.0),
						DDL(w->shadtype, list_shadtype),
						Label(text = "Alignment:", hAlign = 1.0),
						DDL(w->align, list_aligns),
						Label(text = "Item 1:", hAlign = 1.0),
						DDL(w->ctrs[0], list_counters),
						Label(text = "Item 2:", hAlign = 1.0),
						DDL(w->ctrs[1], list_counters),
						Label(text = "Item 3:", hAlign = 1.0),
						DDL(w->ctrs[2], list_counters)
					),
					Column(
						Rows<2>(
							Label(text = "Min Digits:", hAlign = 1.0),
							NUM_FIELD(w->mindigits,0,5),
							Label(text = "Max Digits:", hAlign = 1.0),
							NUM_FIELD(w->maxdigits,0,5),
							Label(text = "Infinite:", hAlign = 1.0),
							DDL_EX(w->infitm,list_items,maxwidth=100_px),
							Label(text = "Inf Character:", hAlign = 1.0),
							TextField(maxLength = 1,
								fitParent = true,
								text = std::string(1,(char)w->infchar),
								onValChangedFunc = [=](GUI::TextField::type,std::string_view str,int32_t)
								{
									std::string txt(str);
									if(txt.size())
										w->infchar = txt[0];
									else w->infchar = 0;
								})
						),
						Rows<2>(
							INFOBTN("If the counter should show while empty"),
							CBOX(w->flags,SUBSCR_COUNTER_SHOW0,"Show Zero",1),
							INFOBTN("If the counter should only show when an item that uses it is equipped to any button"),
							CBOX(w->flags,SUBSCR_COUNTER_ONLYSEL,"Only Selected",1)
						)
					)
				);
				break;
			}
			case widgBTNCOUNTER:
			{
				SW_BtnCounter* w = dynamic_cast<SW_BtnCounter*>(local_subref);
				mergetype = mtFORCE_TAB; //too wide to fit!
				attrib_grid = Row(
					Rows<3>(
						Label(text = "Font:", hAlign = 1.0),
						DDL_FONT(w->fontid),
						_d,
						Label(text = "Style:", hAlign = 1.0),
						DDL(w->shadtype, list_shadtype),
						_d,
						Label(text = "Alignment:", hAlign = 1.0),
						DDL(w->align, list_aligns),
						_d,
						Label(text = "Button:", hAlign = 1.0),
						DDL(w->btn, list_buttons),
						_d,
						Label(text = "Cost:", hAlign = 1.0),
						DDL(w->costind, list_costinds),
						INFOBTN("Which cost to use from the button item")
					),
					Column(
						Rows<2>(
							Label(text = "Min Digits:", hAlign = 1.0),
							NUM_FIELD(w->mindigits,0,5),
							Label(text = "Max Digits:", hAlign = 1.0),
							NUM_FIELD(w->maxdigits,0,5),
							Label(text = "Inf Character:", hAlign = 1.0),
							TextField(maxLength = 1,
								fitParent = true,
								text = std::string(1,(char)w->infchar),
								onValChangedFunc = [=](GUI::TextField::type,std::string_view str,int32_t)
								{
									std::string txt(str);
									if(txt.size())
										w->infchar = txt[0];
									else w->infchar = 0;
								})
						),
						Rows<2>(
							INFOBTN("If the counter should show while empty"),
							CBOX(w->flags,SUBSCR_BTNCOUNTER_SHOW0,"Show Zero",1),
							INFOBTN("If checked, 'Cost: 0' will always check the item's 'Use Cost', and 'Cost: 1' to 'Use Cost 2'.\n"
								"If unchecked, 'Cost: 0' checks the first cost that is not None on an item, whichever that is,"
								" and 'Cost: 2' the second."),
							CBOX(w->flags,SUBSCR_BTNCOUNTER_NOCOLLAPSE,"No Collapse",1)
						)
					)
				);
				break;
			}
			case widgOLDCTR:
			{
				SW_Counters* w = dynamic_cast<SW_Counters*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					Label(text = "Style:", hAlign = 1.0),
					DDL(w->shadtype, list_shadtype),
					Label(text = "Digits:", hAlign = 1.0),
					NUM_FIELD(w->digits,0,5),
					Label(text = "Inf Character:", hAlign = 1.0),
					TextField(maxLength = 1,
						fitParent = true,
						text = std::string(1,(char)w->infchar),
						onValChangedFunc = [=](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string name(str);
							if(name.size())
								w->infchar = name[0];
							else w->infchar = 0;
						}),
					CBOX(w->flags,SUBSCR_COUNTERS_USEX,"Use X",2)
				);
				break;
			}
			case widgITEMSLOT:
			{
				SW_ItemSlot* w = dynamic_cast<SW_ItemSlot*>(local_subref);
				attrib_grid = Rows<3>(
					labels[0] = Label(text = "Item Class:", hAlign = 1.0),
					ddl = DDL(w->iclass, list_itemclass),
					INFOBTN("The highest level owned of this itemclass will be tied to this item slot."),
					Label(text = "Item Override:", hAlign = 1.0),
					DropDownList(data = list_items,
						fitParent = true,
						forceFitW = true,
						selectedValue = w->iid,
						onSelectFunc = [=](int32_t val)
						{
							w->iid = val;
							updateAttr();
						}
					),
					INFOBTN("The specified item ID will be tied to this item slot (OVERRIDES 'Item Class' if set)"),
					//
					Rows<2>(rowSpan = 4, padding = 0_px,
						DEFEQUP_CBOX(0,"A",QRHINT({qr_OLD_SUBSCR,qr_SELECTAWPN})+RULETMPL_HINT({ruletemplateNewSubscreen})),
						DEFEQUP_CBOX(1,"B",QRHINT({qr_OLD_SUBSCR})+RULETMPL_HINT({ruletemplateNewSubscreen})),
						DEFEQUP_CBOX(2,"X",QRHINT({qr_OLD_SUBSCR,qr_SET_XBUTTON_ITEMS})+RULETMPL_HINT({ruletemplateNewSubscreen})),
						DEFEQUP_CBOX(3,"Y",QRHINT({qr_OLD_SUBSCR,qr_SET_YBUTTON_ITEMS})+RULETMPL_HINT({ruletemplateNewSubscreen}))
					),
					//
					CBOX_EX(w->flags,SUBSCR_CURITM_INVIS,"Invisible",_EX_RBOX),
					INFOBTN("If checked, the item is invisible on the subscreen"),
					CBOX_EX(w->flags,SUBSCR_CURITM_NONEQP,"Non-Equippable",_EX_RBOX),
					INFOBTN("If checked, the item cannot be equipped to a button."
						" 'Always Press To Equip' is recommended if this is used." + QRHINT({qr_SUBSCR_PRESS_TO_EQUIP})),
					CBOX_EX(w->flags,SUBSCR_CURITM_IGNR_SP_SELTEXT,"Ignore Special Selection Text",_EX_RBOX),
					INFOBTN("If checked, special selection text (like heart piece count on the"
						" heart piece display, or bow name on bow&arrow) will not be added for this widget."),
					CBOX_EX(w->flags,SUBSCR_CURITM_IGNR_SP_DISPLAY,"Ignore Special Display",_EX_RBOX),
					INFOBTN("If checked, special display properties (like the hardcoded map, compass, boss key"
						" displays and changing heart pieces into the heart piece display) will not occur for this widget."),
					//
					DummyWidget(),
					CBOX_EX(w->flags,SUBSCR_CURITM_NO_INTER_WO_ITEM,"No Interact Without Item",_EX_RBOX),
					INFOBTN("If checked, effects using buttons such as changing pages or running frozen generic scripts"
						" on this widget will not trigger unless there is a valid item displayed."),
					DummyWidget(),
					CBOX_EX(w->flags,SUBSCR_CURITM_NO_INTER_WO_EQUIP,"No Interact Without Equip",_EX_RBOX),
					INFOBTN("If checked, effects using buttons such as changing pages or running frozen generic scripts"
						" on this widget will not trigger unless the buttonpress equips (or intends to equip) the item to a button."),
					DummyWidget(),
					CBOX_EX(w->flags,SUBSCR_CURITM_NO_UNEQUIP,"No Unequip",_EX_RBOX),
					INFOBTN("If checked, trying to equip this widget to a button it is already equipped to will NOT unequip it."
						" Importantly, this also means that with 'No Interact Without Equip' checked, button effects will still"
						" occur when the button is already equipped with this widget.")
				);
				break;
			}
			case widgBGCOLOR:
			{
				break;
			}
			case widgLMAP:
			{
				SW_LMap* w = dynamic_cast<SW_LMap*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_LMAP_SHOWMAP, "Show Map", 1),
					CBOX(w->flags, SUBSCR_LMAP_SHOWROOM, "Show Rooms", 1),
					CBOX(w->flags, SUBSCR_LMAP_SHOWPLR, "Show Hero", 1),
					CBOX(w->flags, SUBSCR_LMAP_LARGE, "Large", 1)
				);
				break;
			}
			case widgLGAUGE:
			case widgMGAUGE:
			case widgMISCGAUGE:
			{
				std::string ctrname;
				switch(widgty)
				{
					case widgLGAUGE:
						ctrname = "HP";
						break;
					case widgMGAUGE:
						ctrname = "MP";
						break;
					case widgMISCGAUGE:
						ctrname = "counter";
						break;
				}
				SW_GaugePiece* w = dynamic_cast<SW_GaugePiece*>(local_subref);
				int g = 0;
				extra_grids["Gauge"] = Row(padding = 0_px,
					Column(padding = 0_px,
						Rows<2>(padding = 0_px,
							Row(colSpan = 2,
								Label(text = "Gauge Tiles"),
								INFOBTN("The tiles used by the gauge. Which tile is used depends on the 'Container' value."
									"\nThe 'Mod' checkbox determines if the tile should be adjusted based on current HP."
										" The tile layout 'Mod' uses is determined by the 'Full Tile' flag and 'Old Gauge Tile Layout'"
										" quest rule."
									"\nIf 'Full Tile' is checked, the layout is as follows: The selected tile appears if the"
										" container has < 'Units per frame' hp, with any animation frames after it. The next tile appears"
										" if the container has < 'Units per frame *2' hp, etc. If the container is the last"
										" full container, and 'Unique Last' is checked, it uses the tile after the full tile."
									"\nIf 'Full Tile' is not checked but 'Old Gauge Tile Layout' is off, it uses the same layout as"
										" the 'Full Tile' layout, but uses minitiles in that order instead."
									"\nIf 'Full Tile' is not checked and 'Old Gauge Tile Layout' is on, then: The full container uses the"
										" first minitile, plus animation frames. The empty container is the top-left minitile of the tile"
										" after the end of the full container, and the layout continues in this way until the just-before-full tile."
										" The last full container with 'Unique Last' checked uses the bottom-right minitile of the tile after the end of this."
									+ QRHINT({qr_OLD_GAUGE_TILE_LAYOUT}))
							),
							GAUGE_MINITILE(0,"Not Last",w->mts[0],w->flags,SUBSCR_GAUGE_MOD1),
							GAUGE_MINITILE(1,"Last",w->mts[1],w->flags,SUBSCR_GAUGE_MOD2),
							GAUGE_MINITILE(2,"Cap",w->mts[2],w->flags,SUBSCR_GAUGE_MOD3),
							GAUGE_MINITILE(3,"After Cap",w->mts[3],w->flags,SUBSCR_GAUGE_MOD4)
						),
						Rows<2>(padding = 0_px,
							labels[3] = Label(text = "Infinite Item:"),
							INFOBTN("Having this item counts as having 'infinite' of this counter,"
								" as far as the infinite-related flags are concerned."),
							ddl = DDL_EX(w->inf_item, list_items, colSpan = 2)
						)
					),
					Column(padding = 0_px,
						Row(padding = 0_px,
							Rows<6>(
								Label(text = "Units per frame:", hAlign = 1.0),
								TextField(
									fitParent = true,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 1, high = 256, val = w->unit_per_frame+1,
									onValChangedFunc = [&, w](GUI::TextField::type,std::string_view,int32_t val)
									{
										w->unit_per_frame = val-1;
										refr_info();
									}),
								INFOBTN("How many points should be allocated to each tile. Ex. If you have 16"
									" HP per container, but only want to show 1/8 hearts, you can use"
									" 2 units per frame (2 = 16/8) and only set up 1/8 heart tiles."),
								Label(text = "Container:", hAlign = 1.0),
								NUM_FIELD(w->container, 0, 9999),
								INFOBTN("The container number this piece represents. For a value of n,"
									"\nIf the Hero has exactly n containers, 'Last' displays."
									"\nIf the Hero has > n containers, 'Not Last' displays."
									"\nIf the Hero has exactly n-1 containers, 'Cap' displays."
									"\nIf the Hero has < n-1 containers, 'After Cap' displays."
									),
								//
								Label(text = "Frames:", hAlign = 1.0),
								TextField(
									fitParent = true,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 1, high = 999, val = w->frames,
									onValChangedFunc = [&, w](GUI::TextField::type,std::string_view,int32_t val)
									{
										w->frames = val;
										refr_info();
									}),
								INFOBTN("Each container tile is an animation of this many frames. 1 = no animation."),
								labels[0] = Label(text = "Anim Cond:", hAlign = 1.0),
								tfs[0] = NUM_FIELD(w->anim_val, 0, 65535),
								INFOBTN("Used by flags 'Animate Only Under...', 'Animate Only Over...' below."),
								//
								labels[1] = Label(text = "Speed:", hAlign = 1.0),
								tfs[1] = NUM_FIELD(w->speed, 1, 999),
								INFOBTN("If animated, the speed at which the animation moves."),
								labels[2] = Label(text = "Delay:", hAlign = 1.0),
								tfs[2] = NUM_FIELD(w->delay, 0, 999),
								INFOBTN("If animated, the delay at each start of the animation."),
								//
								Label(text = "Gauge Width:", hAlign = 1.0),
								TextField(
									fitParent = true,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 1, high = 32, val = w->gauge_wid+1,
									onValChangedFunc = [&, w](GUI::TextField::type,std::string_view,int32_t val)
									{
										w->gauge_wid = val-1;
										refr_info();
									}),
								INFOBTN("If >1, displays multiple gauge pieces width-wise."),
								Label(text = "Gauge Height:", hAlign = 1.0),
								TextField(
									fitParent = true,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 1, high = 32, val = w->gauge_hei+1,
									onValChangedFunc = [&, w](GUI::TextField::type,std::string_view,int32_t val)
									{
										w->gauge_hei = val-1;
										refr_info();
									}),
								INFOBTN("If >1, displays multiple gauge pieces height-wise."),
								//
								Label(text = "HSpace:", hAlign = 1.0),
								gauge_gw[g++] = NUM_FIELD(w->hspace, -128, 127),
								INFOBTN("Extra space between gauge pieces width-wise."),
								Label(text = "VSpace:", hAlign = 1.0),
								gauge_gw[g++] = NUM_FIELD(w->vspace, -128, 127),
								INFOBTN("Extra space between gauge pieces height-wise."),
								//
								Label(text = "Grid XOffset:", hAlign = 1.0),
								gauge_gw[g++] = NUM_FIELD(w->grid_xoff, -999, 999),
								INFOBTN("Each row is offset by this much x."),
								Label(text = "Grid YOffset:", hAlign = 1.0),
								gauge_gw[g++] = NUM_FIELD(w->grid_yoff, -999, 999),
								INFOBTN("Each column is offset by this much y.")
							)
						),
						Row(padding = 0_px,
							Rows<2>(vAlign = 0.0,
								INFOBTN("Animates only when "+ctrname+" is <= 'Anim Cond'"),
								cbs[0] = Checkbox(
									text = "Animate Only Under...", hAlign = 0.0,
									checked = w->flags & SUBSCR_GAUGE_ANIM_UNDER,
									onToggleFunc = [&, w](bool state)
									{
										SETFLAG(w->flags, SUBSCR_GAUGE_ANIM_UNDER, state);
										refr_info();
									}
								),
								INFOBTN("Animates only when "+ctrname+" is >= 'Anim Cond'"),
								cbs[1] = Checkbox(
									text = "Animate Only Over...", hAlign = 0.0,
									checked = w->flags & SUBSCR_GAUGE_ANIM_OVER,
									onToggleFunc = [&, w](bool state)
									{
										SETFLAG(w->flags, SUBSCR_GAUGE_ANIM_OVER, state);
										refr_info();
									}
								),
								INFOBTN("The 'Anim Cond' value is treated as a percentage of max "+ctrname+"."),
								cbs[2] = CBOX(w->flags,SUBSCR_GAUGE_ANIM_PERCENT,"Anim Cond is Percent",1),
								INFOBTN("When animating, skip the first frame."),
								cbs[3] = Checkbox(
									text = "Skip First Frame", hAlign = 0.0,
									checked = w->flags & SUBSCR_GAUGE_ANIM_SKIP,
									onToggleFunc = [&, w](bool state)
									{
										SETFLAG(w->flags, SUBSCR_GAUGE_ANIM_SKIP, state);
										refr_info();
									}
								),
								INFOBTN("Will be invisible if infinite of the counter IS possessed."
									" Counts owning the 'Infinite Item', or hardcoded infinities like"
									" wallets granting infinite money."),
								Checkbox(
									text = "No Infinite", hAlign = 0.0,
									checked = w->flags & SUBSCR_GAUGE_INFITM_BAN,
									onToggleFunc = [&, w](bool state)
									{
										SETFLAG(w->flags, SUBSCR_GAUGE_INFITM_BAN, state);
										refr_info();
									}
								),
								INFOBTN("Will be invisible if infinite of the counter IS NOT possessed."
									" Counts owning the 'Infinite Item', or hardcoded infinities like"
									" wallets granting infinite money."),
								Checkbox(
									text = "Require Infinite", hAlign = 0.0,
									checked = w->flags & SUBSCR_GAUGE_INFITM_REQ,
									onToggleFunc = [&, w](bool state)
									{
										SETFLAG(w->flags, SUBSCR_GAUGE_INFITM_REQ, state);
										refr_info();
									}
								)
							),
							Rows<2>(vAlign = 0.0,
								INFOBTN("If a unique tile should be used for the last container when full."),
								CBOX(w->flags,SUBSCR_GAUGE_UNQLAST,"Unique Last",1),
								INFOBTN("Use full tiles instead of mini-tiles"),
								Checkbox(
									text = "Full Tile", hAlign = 0.0,
									checked = w->flags & SUBSCR_GAUGE_FULLTILE,
									onToggleFunc = [&, w](bool state)
									{
										SETFLAG(w->flags, SUBSCR_GAUGE_FULLTILE, state);
										refr_info();
									}
								),
								INFOBTN("If the grid order should go right-to-left instead of left-to-right"),
								gauge_gw[g++] = CBOX(w->flags, SUBSCR_GAUGE_GRID_RTOL, "Grid Right to Left", 1),
								INFOBTN("If the grid order should go top-to-bottom instead of bottom-to-top"),
								gauge_gw[g++] = CBOX(w->flags, SUBSCR_GAUGE_GRID_TTOB, "Grid Top to Bottom", 1),
								INFOBTN("If the grid should grow in the columns before the rows"),
								gauge_gw[g++] = CBOX(w->flags, SUBSCR_GAUGE_GRID_COLUMN1ST, "Grid Column First", 1),
								INFOBTN("If the grid should alternate direction when finishing a row (or column)"),
								gauge_gw[g++] = CBOX(w->flags, SUBSCR_GAUGE_GRID_SNAKE, "Grid Snake Pattern", 1)
							)
						)
					)
				);
				switch(widgty)
				{
					case widgLGAUGE:
						break; //nothing else
					case widgMGAUGE:
					{
						SW_MagicGaugePiece* w = dynamic_cast<SW_MagicGaugePiece*>(local_subref);
						attrib_grid = Row(
							Label(text = "Show when Magic Drain =", hAlign = 1.0),
							NUM_FIELD(w->showdrain, -1, 9999),
							INFOBTN("If set to -1, piece shows normally."
								"\nIf set to > -1, piece will only be visible when your"
								" Magic Drain Rate is equal to the value."
								"\nEx. With the default of starting at drain rate 2,"
								" a show value of '1' would be used for a '1/2 magic' icon."
								"\nWith a starting drain rate of 4, you could"
								" then have a show value of '2' for a '1/2 magic', and a"
								" show value of '1' for a '1/4 magic'.")
						);
						break;
					}
					case widgMISCGAUGE:
					{
						SW_MiscGaugePiece* w = dynamic_cast<SW_MiscGaugePiece*>(local_subref);
						attrib_grid = Rows<3>(
							Label(text = "Counter", hAlign = 1.0),
							DDL(w->counter, list_counters),
							INFOBTN("The counter for this gauge."),
							Label(text = "Per Container", hAlign = 1.0),
							NUM_FIELD(w->per_container, 1, 65535),
							INFOBTN("The number of the counter 'per container' of the gauge.")
						);
						break;
					}
				}
				
				break;
			}
			case widgLMETER:
			{
				SW_LifeMeter* w = dynamic_cast<SW_LifeMeter*>(local_subref);
				attrib_grid = Rows<2>(
					Label(text = "Rows:", hAlign = 1.0),
					DDL(w->rows, two_three_rows),
					CBOX(w->flags, SUBSCR_LIFEMET_BOT, "Start at bottom", 2)
				);
				break;
			}
			case widgLINE:
			{
				SW_Line* w = dynamic_cast<SW_Line*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_LINE_TRANSP, "Transparent", 1)
				);
				break;
			}
			case widgMMETER:
			{
				break;
			}
			case widgMMAP:
			{
				SW_MMap* w = dynamic_cast<SW_MMap*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Row(
					Rows<2>(
						CBOX(w->flags, SUBSCR_MMAP_SHOWMAP, "Show Map", 1),
						INFOBTN("Show the map itself. If unchecked, only the markers for 'Show Hero' and 'Show Compass' will be drawn."),
						CBOX(w->flags, SUBSCR_MMAP_SHOWPLR, "Show Hero", 1),
						INFOBTN("Show the hero's current position on the map."),
						CBOX(w->flags, SUBSCR_MMAP_SHOWCMP, "Show Compass", 1),
						INFOBTN("Show the compass marker, which points to the player's destination. Will blink between two colors until"
							" all of the specified level items are collected.")
					),
					Frame(title = "Compass Blink Stops",
						info = "The compass marker will stop blinking when all of these are collected for the current level",
						Columns<4>(
							CBOX(w->compass_litems, liTRIFORCE, "McGuffin", 1),
							CBOX(w->compass_litems, liMAP, "Map", 1),
							CBOX(w->compass_litems, liCOMPASS, "Compass", 1),
							CBOX(w->compass_litems, liBOSS, "Boss Killed", 1),
							CBOX(w->compass_litems, liBOSSKEY, "Boss Key", 1),
							CBOX(w->compass_litems, liCUSTOM01, "Custom 01", 1),
							CBOX(w->compass_litems, liCUSTOM02, "Custom 02", 1),
							CBOX(w->compass_litems, liCUSTOM03, "Custom 03", 1)
						)
					)
				);
				break;
			}
			case widgMMAPTITLE:
			{
				SW_MMapTitle* w = dynamic_cast<SW_MMapTitle*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					Label(text = "Style:", hAlign = 1.0),
					DDL(w->shadtype, list_shadtype),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(w->align, list_aligns),
					Label(text = "Tabsize:", hAlign = 0.0),
					NUM_FIELD(w->tabsize, 0, 99),
					CBOX(w->flags, SUBSCR_MMAPTIT_WORDWRAP, "Word Wrap", 2),
					CBOX(w->flags, SUBSCR_MMAPTIT_REQMAP, "Invisible w/o Map item", 2),
					CBOX(w->flags, SUBSCR_MMAPTIT_ONELINE, "Show as one line", 2)
				);
				break;
			}
			case widgMINITILE:
			{
				SW_MiniTile* w = dynamic_cast<SW_MiniTile*>(local_subref);
				mergetype = mtLOCTOP; //too wide to fit all 3 horiz, but has vert room
				int32_t tl, tw = 1, crn;
				if(w->tile == -1)
				{
					switch(w->special_tile)
					{
						case ssmstSSVINETILE:
							tl = wpnsbuf[iwSubscreenVine].tile;
							tw = 3;
							crn = w->crn;
							break;
						case ssmstMAGICMETER:
							tl = wpnsbuf[iwMMeter].tile;
							tw = 9;
							crn = w->crn;
							break;
					}
				}
				else
				{
					tl = w->tile;
					crn = w->crn;
					w->special_tile = -1;
				}
				attrib_grid = Rows<2>(
					Label(text = "Tile:", hAlign = 1.0),
					tswatches[0] = SelTileSwatch(
						width = 4_px + (32_px*9),
						hAlign = 0.0,
						tile = tl,
						cset = w->cs.get_cset(),
						tilewid = tw,
						minionly = w->special_tile != -1,
						mini = true,
						minicorner = crn,
						showvals = false,
						showT0 = true,
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t crn)
						{
							if(w->special_tile != -1)
							{
								w->crn = crn;
								return;
							}
							
							if(w->cs.type != ssctMISC)
							{
								w->cs.type = c;
								cs_sel[0]->setC1(c);
							}
							w->tile = t;
							w->crn = crn;
						}
					),
					Label(text = "Special Tile:", hAlign = 1.0),
					ddl = DropDownList(data = special_tile_list,
						fitParent = true,
						selectedValue = w->special_tile,
						onSelectFunc = [=](int32_t val)
						{
							if(val == w->special_tile) return;
							auto oldval = w->special_tile;
							w->special_tile = val;
							int32_t newtile = 0;
							switch(val)
							{
								case -1:
									tswatches[0]->setTileWid(1);
									tswatches[0]->setMiniOnly(false);
									newtile = std::max(0,w->tile);
									w->tile = newtile;
									break;
								case 0:
									tswatches[0]->setTileWid(3);
									tswatches[0]->setMiniOnly(true);
									newtile = wpnsbuf[iwSubscreenVine].tile;
									w->tile = newtile;
									break;
								case 1:
									tswatches[0]->setTileWid(9);
									tswatches[0]->setMiniOnly(true);
									newtile = wpnsbuf[iwMMeter].tile;
									w->tile = newtile;
									break;
							}
							tswatches[0]->setTile(newtile);
							tswatches[0]->setMiniCrn(w->crn);
						}
					),
					CBOX(w->flags, SUBSCR_MINITL_OVERLAY, "Overlay", 2),
					CBOX(w->flags, SUBSCR_MINITL_TRANSP, "Transparent", 2)
				);
				break;
			}
			case widgRECT:
			{
				SW_Rect* w = dynamic_cast<SW_Rect*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_RECT_FILLED, "Filled", 1),
					CBOX(w->flags, SUBSCR_RECT_TRANSP, "Transparent", 1)
				);
				break;
			}
			case widgSELECTEDTEXT:
			{
				SW_SelectedText* w = dynamic_cast<SW_SelectedText*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					Label(text = "Style:", hAlign = 1.0),
					DDL(w->shadtype, list_shadtype),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(w->align, list_aligns),
					Label(text = "Tabsize:", hAlign = 0.0),
					NUM_FIELD(w->tabsize,0,99),
					//
					CBOX(w->flags,SUBSCR_SELTEXT_WORDWRAP,"Word Wrap",2)
				);
				break;
			}
			case widgSELECTOR:
			{
				SW_Selector* w = dynamic_cast<SW_Selector*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_SELECTOR_TRANSP, "Transparent", 1),
					CBOX(w->flags, SUBSCR_SELECTOR_LARGE, "Large", 1),
					CBOX(w->flags, SUBSCR_SELECTOR_USEB, "Use Selector 2", 1)
				);
				break;
			}
			case widgTEXT:
			{
				SW_Text* w = dynamic_cast<SW_Text*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					Label(text = "Style:", hAlign = 1.0),
					DDL(w->shadtype, list_shadtype),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(w->align, list_aligns),
					Label(text = "Text:", hAlign = 0.0),
					DummyWidget(),
					fonttf = TextField(
						maxLength = 256,
						colSpan = 2,
						text = w->text,
						width = 300_px,
						minheight = 15_px,
						fitParent = true,
						useFont = get_zc_font(w->fontid),
						onValChangedFunc = [=](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string txt(str);
							w->text = txt;
						})
				);
				break;
			}
			case widgTEXTBOX:
			{
				SW_TextBox* w = dynamic_cast<SW_TextBox*>(local_subref);
				mergetype = mtFORCE_TAB;
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					Label(text = "Style:", hAlign = 1.0),
					DDL(w->shadtype, list_shadtype),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(w->align, list_aligns),
					Label(text = "Tabsize:", hAlign = 0.0),
					NUM_FIELD(w->tabsize,0,99),
					//
					CBOX(w->flags,SUBSCR_TEXTBOX_WORDWRAP,"Word Wrap",2),
					//
					Label(text = "Text:", hAlign = 0.0),
					DummyWidget(),
					fonttf = TextField(
						maxLength = 1024,
						colSpan = 2,
						text = w->text,
						width = 300_px,
						minheight = 15_px,
						fitParent = true,
						useFont = get_zc_font(w->fontid),
						onValChangedFunc = [=](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string txt(str);
							w->text = txt;
						})
				);
				break;
			}
			case widgTILEBLOCK:
			{
				SW_TileBlock* w = dynamic_cast<SW_TileBlock*>(local_subref);
				mergetype = mtLOCTOP; //too wide to fit all 3 horiz, but has vert room
				attrib_grid = Column(
					tswatches[0] = SelTileSwatch(
						hAlign = 0.0,
						minwidth = 32_px*TB_LA+4_px,
						minheight = 32_px*TB_LA+4_px,
						tile = w->tile,
						cset = w->cs.get_cset(),
						showvals = false,
						tilewid = std::min(local_subref->w, (word)TB_LA),
						tilehei = std::min(local_subref->h, (word)TB_LA),
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t)
						{
							w->tile = t;
							if(w->cs.type != ssctMISC)
							{
								w->cs.type = c;
								cs_sel[0]->setC1(c);
							}
						}
					),
					Label(text = "Note: Preview max size is " + std::to_string(TB_LA) + "x" + std::to_string(TB_LA)),
					CBOX(w->flags, SUBSCR_TILEBL_OVERLAY, "Overlay", 1),
					CBOX(w->flags, SUBSCR_TILEBL_TRANSP, "Transparent", 1)
				);
				break;
			}
			case widgMCGUFF_FRAME:
			{
				SW_TriFrame* w = dynamic_cast<SW_TriFrame*>(local_subref);
				mergetype = mtFORCE_TAB; //Way too wide to fit
				attrib_grid = Columns<3>(padding = 0_px,
					Label(text = "Frame Tileblock"),
					tswatches[0] = SelTileSwatch(
						hAlign = 0.0,
						rowSpan = 2,
						minwidth = 32_px*7+4_px,
						minheight = 32_px*7+4_px,
						tile = w->frame_tile,
						cset = w->frame_cset,
						showvals = false,
						tilewid = (w->flags&SUBSCR_TRIFR_LGPC) ? 7 : 6,
						tilehei = (w->flags&SUBSCR_TRIFR_LGPC) ? 7 : 3,
						deftile = QMisc.colors.triframe_tile,
						defcs = QMisc.colors.triframe_cset,
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t)
						{
							w->frame_tile = t;
							w->frame_cset = c;
						}
					),
					Label(text = "Piece Tile"),
					tswatches[1] = SelTileSwatch(
						hAlign = 0.0,
						minwidth = 32_px*2+4_px,
						minheight = 32_px*3+4_px,
						tile = w->piece_tile,
						cset = w->piece_cset,
						showvals = false,
						tilewid = (w->flags&SUBSCR_TRIFR_LGPC) ? 2 : 1,
						tilehei = (w->flags&SUBSCR_TRIFR_LGPC) ? 3 : 1,
						deftile = QMisc.colors.triforce_tile,
						defcs = QMisc.colors.triforce_cset,
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t)
						{
							w->piece_tile = t;
							w->piece_cset = c;
						}
					),
					Column(padding = 0_px,
						colSpan = 2,
						labels[0] = Label(text = "Tile 0 uses a preset from"
							"\n'Quest->Graphics->Map Styles'"),
						CBOX(w->flags,SUBSCR_TRIFR_SHOWFR,"Show Frame",1),
						CBOX(w->flags,SUBSCR_TRIFR_SHOWPC,"Show Pieces",1),
						Checkbox(
							text = "Large Pieces", hAlign = 0.0,
							checked = w->flags & SUBSCR_TRIFR_LGPC,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(w->flags, SUBSCR_TRIFR_LGPC, state);
								tswatches[0]->setTileWid(state ? 7 : 6);
								tswatches[0]->setTileHei(state ? 7 : 3);
								tswatches[1]->setTileWid(state ? 2 : 1);
								tswatches[1]->setTileHei(state ? 3 : 1);
							}
						)
					)
				);
				break;
			}
			case widgMCGUFF:
			{
				SW_McGuffin* w = dynamic_cast<SW_McGuffin*>(local_subref);
				mergetype = mtLOCTOP;
				attrib_grid = Rows<2>(padding = 0_px,
					Label(text = "Tile:", hAlign = 1.0),
					tswatches[0] = SelTileSwatch(
						hAlign = 0.0,
						tile = w->tile,
						cset = w->cs.get_cset(),
						showvals = false,
						// tilewid = local_subref->d7 ? 2 : 1,
						// tilehei = local_subref->d7 ? 3 : 1,
						deftile = QMisc.colors.triforce_tile,
						defcs = QMisc.colors.triforce_cset,
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t)
						{
							w->tile = t;
							if(w->cs.type != ssctMISC)
							{
								w->cs.type = c;
								cs_sel[0]->setC1(c);
							}
						}
					),
					Column(padding = 0_px,
						colSpan = 2,
						labels[0] = Label(text = "Tile 0 uses a preset from"
							"\n'Quest->Graphics->Map Styles'"),
						CBOX(w->flags,SUBSCR_MCGUF_OVERLAY,"Overlay",1),
						CBOX(w->flags,SUBSCR_MCGUF_TRANSP,"Transparent",1)
					),
					Label(text = "Piece #:", hAlign = 1.0),
					NUM_FIELD(w->number,0,999)
				);
				break;
			}
			case widgCOUNTERPERCBAR:
			{
				SW_CounterPercentBar* w = dynamic_cast<SW_CounterPercentBar*>(local_subref);
				attrib_grid = Rows<3>(
					Label(text = "Counter", hAlign = 1.0),
					DDL(w->counter, list_counters),
					INFOBTN("The counter for this percentage bar."),
					CBOX(w->flags, SUBSCR_COUNTERPERCBAR_TRANSP, "Transparent", 2),
					INFOBTN("If checked, the bar is drawn transparently (both empty and full parts)"),
					CBOX(w->flags, SUBSCR_COUNTERPERCBAR_VERTICAL, "Vertical", 2),
					INFOBTN("If checked, the bar is filled vertically instead of horizontally"),
					CBOX(w->flags, SUBSCR_COUNTERPERCBAR_INVERT, "Invert", 2),
					INFOBTN("If checked, the bar fill direction is reversed (top-to-bottom or right-to-left)")
				);
				
				break;
			}
			default: attrib_grid = Column(Label(text = "ERROR")); break;
		}
	}
	
	std::shared_ptr<GUI::Grid> g = Rows<2>(padding = 0_px);
	std::shared_ptr<GUI::TabPanel> tpan = TabPanel(ptr = &sprop_tabs[local_subref->getType()],
			TabRef(name = "Basic", g)
		);
	switch(mergetype)
	{
		default:
		case mtNONE: //3 in horz row
		{
			g->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(col_grid)
				g->add(Frame(title = "Color", fitParent = true, col_grid));
			if(attrib_grid)
				g->add(Frame(title = "Attributes", fitParent = true,
					colSpan = col_grid ? 2 : 1, attrib_grid));
			break;
		}
		case mtFORCE_TAB: //3 separate tabs
		{
			g->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(col_grid)
				g->add(Frame(title = "Color", fitParent = true, col_grid));
			if(attrib_grid)
				tpan->add(TabRef(name = "Attributes", attrib_grid));
			break;
		}
		case mtLOCTOP:
		{
			if(col_grid)
			{
				g->add(Column(padding = 0_px,
					Frame(title = "Location", fitParent = true, loc_grid),
					Frame(title = "Color", fitParent = true, col_grid)));
			}
			else g->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(attrib_grid)
				g->add(Frame(title = "Attributes", fitParent = true, attrib_grid));
			break;
		}
	}
	extra_grids.build(tpan);
	tpan->add(TabRef(name = "Selection",
		TabPanel(ptr = &sprop_tab_sel,
			TabRef(name = "Basic",
				Rows<2>(
					Frame(title = "Cursor Selectable", info = "If the subscreen cursor can select this"
						" widget, and the selectable position information for selecting it.",
						fitParent = true,
						Column(padding = 0_px,
							Checkbox(
								text = "Is Selectable",
								checked = local_subref->genflags & SUBSCRFLAG_SELECTABLE,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_subref->genflags, SUBSCRFLAG_SELECTABLE, state);
									updateSelectable();
								}),
							selgs[0] = Rows<3>(
								Label(text = "Position:", hAlign = 1.0),
								NUM_FIELD(local_subref->pos, 0, 254),
								INFOBTN("The unique position ID of this slot. Setting this to a number"
									" that is already in use on this page may cause buggy behavior."),
								Label(text = "Up Select:", hAlign = 1.0),
								NUM_FIELD(local_subref->pos_up, 0, 254),
								INFOBTN("The unique position ID to move to when pressing 'Up'"),
								Label(text = "Down Select:", hAlign = 1.0),
								NUM_FIELD(local_subref->pos_down, 0, 254),
								INFOBTN("The unique position ID to move to when pressing 'Down'"),
								Label(text = "Left Select:", hAlign = 1.0),
								NUM_FIELD(local_subref->pos_left, 0, 254),
								INFOBTN("The unique position ID to move to when pressing 'Left' / 'L' quickswap"),
								Label(text = "Right Select:", hAlign = 1.0),
								NUM_FIELD(local_subref->pos_right, 0, 254),
								INFOBTN("The unique position ID to move to when pressing 'Right' / 'R' quickswap")
							)
						)
					),
					selframes[0] = Frame(title = "Page Change",
						info = "Change the current subscreen page when a button is pressed.",
							fitParent = true,
							Column(
								Rows<3>(
									Label(text = "Mode:",hAlign = 1.0),
									DropDownList(data = list_pgmode,
										fitParent = true, selectedValue = local_subref->pg_mode,
										onSelectFunc = [&](int32_t val)
										{
											local_subref->pg_mode = val;
											updateSelectable();
										}),
									INFOBTN("Which mode to use for page swapping."
										"\nPrev/Next move to the previous or next page."
										"\nTarget moves to a specific page number."
										"\nNone disables page swapping altogether."),
									//
									Label(text = "Target:",hAlign = 1.0),
									seltfs[0] = TextField(
										fitParent = true,
										type = GUI::TextField::type::INT_DECIMAL,
										low = 1, high = MAX_SUBSCR_PAGES, val = local_subref->pg_targ+1,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_subref->pg_targ = val-1;
										}),
									INFOBTN("The target page for 'Target' mode."
										" If set to an invalid page, no page change occurs."),
									//
									Label(text = "Transition Effects:"),
									selbtns[0] = Button(text = "Edit",
										maxheight = 2_em,
										onPressFunc = [&]()
										{
											call_subscrtransition_dlg(local_subref->pg_trans,"Transition Editor: Page Change FX");
										}),
									DummyWidget(),
									//
									DummyWidget(),
									CBOX(local_subref->genflags,SUBSCRFLAG_PGGOTO_NOWRAP,"No Wrap",1),
									INFOBTN("If checked, trying to page left from page 0 or"
										" right from the final page will do nothing.")
								),
								INTBTN_PANEL2(local_subref->pg_btns,"Page Button:")
							)
						),
					selframes[1] = Frame(title = "Selection Text",
						info = "If not blank, use this text for 'Selection Text' widgets.",
							fitParent = true, colSpan = 2,
							TextField(
								type = GUI::TextField::type::TEXT,
								maxLength = 255,
								text = local_subref->override_text,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view text,int32_t)
								{
									local_subref->override_text = text;
								})
						)
				)
			),
			TabRef(name = "Selector",
				selframes[2] = Frame(title = "Selector Customization",
					info = "Customize the selector cursor",
					fitParent = true,
					Column(
						Rows<2>(
							CBOX_EX(local_subref->genflags,SUBSCRFLAG_SELOVERRIDE,"Override Selector",
								_EX_RBOX,onToggle = message::REFR_SELECTABLE),
							INFOBTN("Change the selector while over this widget")
						),
						selgs[3] = Row(
							Frame(title = "Dimensions", fitParent = true,
								Rows<3>(
									Label(text = "Selector X", hAlign = 1.0),
									NUM_FIELD(local_subref->selector_override.x,0,255),
									INFOBTN("The top-left of the selector will be forced to this X"),
									Label(text = "Selector Y", hAlign = 1.0),
									NUM_FIELD(local_subref->selector_override.y,0,255),
									INFOBTN("The top-left of the selector will be forced to this Y"),
									Label(text = "Selector Width", hAlign = 1.0),
									NUM_FIELD(local_subref->selector_override.w,0,256),
									INFOBTN("The selector will be forced to this width"),
									Label(text = "Selector Height", hAlign = 1.0),
									NUM_FIELD(local_subref->selector_override.h,0,256),
									INFOBTN("The selector will be forced to this height")
								)),
							Frame(title = "Selector 1 Graphic", fitParent = true,
								info = "Override the Tile, CSet, and Width/Height used by selectors"
									" with 'Use Selector 2' unchecked."
									"\nWidth/Height are given in pixels, and that pixel size will be"
									" used as the source size of the draw. These sizes rounded up to the"
									" next full tile will be the tile width/height of the draw.",
								SELECTOR_GRAPHIC(local_subref->selector_override.tileinfo[0])
							),
							Frame(title = "Selector 2 Graphic", fitParent = true,
								info = "Override the Tile, CSet, and Width/Height used by selectors"
									" with 'Use Selector 2' checked."
									"\nWidth/Height are given in pixels, and that pixel size will be"
									" used as the source size of the draw. These sizes rounded up to the"
									" next full tile will be the tile width/height of the draw.",
								SELECTOR_GRAPHIC(local_subref->selector_override.tileinfo[1])
							)
						)
					)
				)
			),
			seltabs[0] = TabRef(name = "Script",
				Frame(title = "Generic Frozen Script",
					info = "Run a Generic Frozen Script when a button is pressed."
						"\nThe script will run before any other effects that occur from"
						" pressing the button (such as equipping an item to a button).",
					Row(
						selgs[1] = Column(padding = 0_px,
							GEN_INITD(0),
							GEN_INITD(1),
							GEN_INITD(2),
							GEN_INITD(3),
							GEN_INITD(4),
							GEN_INITD(5),
							GEN_INITD(6),
							GEN_INITD(7)
						),
						Column(
							Label(text = "Script:", hAlign = 0.0),
							DropDownList(data = list_genscr,
								vPadding = 0_px,
								fitParent = true, selectedValue = local_subref->generic_script,
								onSelectFunc = [&](int32_t val)
								{
									local_subref->generic_script = val;
									updateSelectable();
								}),
							selgs[2] = INTBTN_PANEL2(local_subref->gen_script_btns,"Run Button:")
						)
					)
				)
			)
		)));
	cond_item_sels[CI_REQ] = local_subref->req_owned_items.empty() ? -1 : *(local_subref->req_owned_items.begin());
	cond_item_sels[CI_PICKED] = 0;
	cond_item_sels[CI_REQ_NOT] = local_subref->req_unowned_items.empty() ? -1 : *(local_subref->req_unowned_items.begin());
	updateConditions();
	std::shared_ptr<GUI::List> cond_itms_list;
	tpan->add(TabRef(name = "Conditions",
		TabPanel(
			TabRef(name = "Items",
				Columns<4>(
					Row(padding = 0_px,
						Label(text = "Required Owned Items"),
						INFOBTN("Widget will not exist unless these items are owned.")
					),
					req_item_list = List(minheight = 100_px,
						data = list_reqitems,
						focused = true,
						selectedValue = cond_item_sels[CI_REQ],
						onSelectFunc = [&](int32_t val)
						{
							cond_item_sels[CI_REQ] = val;
						}
					),
					Row(padding = 0_px,
						Label(text = "Required Unowned Items"),
						INFOBTN("Widget will not exist if any of these items are owned.")
					),
					req_not_item_list = List(minheight = 100_px,
						data = list_reqnotitems,
						focused = true,
						selectedValue = cond_item_sels[CI_REQ_NOT],
						onSelectFunc = [&](int32_t val)
						{
							cond_item_sels[CI_REQ_NOT] = val;
						}
					),
					Column(rowSpan = 2,
						Button(text = "->",
							onPressFunc = [&]()
							{
								local_subref->req_owned_items.erase(cond_item_sels[CI_REQ]);
								updateConditions();
							}),
						Button(text = "<-",
							onPressFunc = [&]()
							{
								local_subref->req_owned_items.insert(cond_item_sels[CI_PICKED]);
								updateConditions();
							})
					),
					Column(rowSpan = 2,
						Button(text = "->",
							onPressFunc = [&]()
							{
								local_subref->req_unowned_items.erase(cond_item_sels[CI_REQ_NOT]);
								updateConditions();
							}),
						Button(text = "<-",
							onPressFunc = [&]()
							{
								local_subref->req_unowned_items.insert(cond_item_sels[CI_PICKED]);
								updateConditions();
							})
					),
					Label(text = "Items"),
					cond_itms_list = List(minheight = 300_px, fitParent = true, rowSpan = 3,
						data = list_items_no_none.filter([&](GUI::ListItem& itm)
							{
								if(itm.value < 0 || itm.value >= MAXITEMS)
									return false;
								itemdata const& idata = itemsbuf[itm.value];
								return bool(idata.flags & item_gamedata); // only ownable items are usable
							}),
						isABC = true,
						focused = true,
						selectedIndex = 0,
						onSelectFunc = [&](int32_t val)
						{
							cond_item_sels[CI_PICKED] = val;
						}
					)
				)
			),
			TabRef(name = "Other",
				Column(
					Frame(title = "Counter Requirement", info = "If either the counter or the operator is 'None', no counter will be checked."
						" If 'Percentage Value' is checked, the percent of total of the selected counter will be compared;"
						" else if 'Max Value' is checked, the max value of the selected counter will be compared. If neither is checked,"
						" the current value of the counter will be compared.",
						Column(
							Row(
								DropDownList(data = list_counters,
									fitParent = true,
									selectedValue = local_subref->req_counter,
									onSelectFunc = [&](int32_t val)
									{
										local_subref->req_counter = val;
									}),
								DropDownList(data = list_condty,
									fitParent = true,
									selectedValue = local_subref->req_counter_cond_type,
									onSelectFunc = [&](int32_t val)
									{
										local_subref->req_counter_cond_type = val;
									}),
								TextField(
									fitParent = true,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 0, high = 65535, val = local_subref->req_counter_val,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_subref->req_counter_val = val;
									})
							),
							Row(
								Checkbox(
									text = "Max Value", hAlign = 0.0,
									checked = local_subref->genflags & SUBSCRFLAG_REQ_COUNTER_MAX,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_subref->genflags,SUBSCRFLAG_REQ_COUNTER_MAX,state);
									}),
								Checkbox(
									text = "Percentage Value", hAlign = 0.0,
									checked = local_subref->genflags & SUBSCRFLAG_REQ_COUNTER_PERC,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_subref->genflags,SUBSCRFLAG_REQ_COUNTER_PERC,state);
									})
							)
						)
					),
					Frame(title = "Level Items", info = "All checked LItems are required, unless 'Invert' is checked, then it is required to have none instead.",
						Row(
							Rows_Columns<2, 4>(
								INFOBTN("The Hero has the McGuffin for the specified level"),
								CBOX(local_subref->req_litems, liTRIFORCE, "McGuffin", 1),
								INFOBTN("The Hero has the Map for the specified level"),
								CBOX(local_subref->req_litems, liMAP, "Map", 1),
								INFOBTN("The Hero has the Compass for the specified level"),
								CBOX(local_subref->req_litems, liCOMPASS, "Compass", 1),
								INFOBTN("The Hero has cleared the 'Dungeon Boss' room for the specified level"),
								CBOX(local_subref->req_litems, liBOSS, "Boss Killed", 1),
								INFOBTN("The Hero has the Boss Key for the specified level"),
								CBOX(local_subref->req_litems, liBOSSKEY, "Boss Key", 1),
								INFOBTN("The Hero has the 'Custom 01' for the specified level"),
								CBOX(local_subref->req_litems, liCUSTOM01, "Custom 01", 1),
								INFOBTN("The Hero has the 'Custom 02' for the specified level"),
								CBOX(local_subref->req_litems, liCUSTOM02, "Custom 02", 1),
								INFOBTN("The Hero has the 'Custom 03' for the specified level"),
								CBOX(local_subref->req_litems, liCUSTOM03, "Custom 03", 1)
							),
							Rows<3>(
								Label(text = "For Level:"),
								TextField(
									fitParent = true,
									vPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									low = -1, high = MAXLEVELS, val = local_subref->req_litem_level,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_subref->req_litem_level = val;
									}),
								INFOBTN("The required litems will be for this specified level. Specifying '-1' will require the items"
									" for the *current* level."),
								Checkbox(
									text = "Invert", hAlign = 1.0, colSpan = 2,
									boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
									checked = local_subref->genflags & SUBSCRFLAG_REQ_INVERT_LITEM,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_subref->genflags,SUBSCRFLAG_REQ_INVERT_LITEM,state);
									}
								),
								INFOBTN("If checked, it will be required to have NONE of the LItems instead of ALL of the LItems.")
							)
						)
					)
				)
			)
		)
	));
	tpan->add(TabRef(name = "Script",
		Row(
			Label(text = "Label:"),
			TextField(
				type = GUI::TextField::type::TEXT,
				maxLength = 300, maxwidth = 25_em,
				text = local_subref->label,
				onValChangedFunc = [&](GUI::TextField::type,std::string_view text,int32_t)
				{
					local_subref->label = text;
				}),
			INFOBTN("A label scripts can use to find this widget")
		)));
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		Column(
			tpan,
			Row(
				Button(
					text = "&OK",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::OK,
					focused = true),
				Button(
					text = "&Cancel",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	cond_item_sels[CI_PICKED] = cond_itms_list->getSelectedValue();
	updateSelectable();
	updateAttr();
	refr_info();
	return window;
}

void SubscrPropDialog::updateSelectable()
{
	bool seldis = !(local_subref->genflags & SUBSCRFLAG_SELECTABLE);
	bool scrdis = seldis || !local_subref->generic_script;
	bool selovdis = seldis || !(local_subref->genflags & SUBSCRFLAG_SELOVERRIDE);
	bool pgdis = seldis || !local_subref->pg_mode;
	selgs[0]->setDisabled(seldis);
	selframes[0]->setDisabled(seldis);
	selframes[1]->setDisabled(seldis);
	selframes[2]->setDisabled(seldis);
	seltabs[0]->setDisabled(seldis);
	selgs[1]->setDisabled(scrdis);
	selgs[2]->setDisabled(scrdis);
	selgs[3]->setDisabled(selovdis);
	seltfs[0]->setDisabled(pgdis || local_subref->pg_mode != PGGOTO_TRG);
	selbtns[0]->setDisabled(pgdis);
	
	
	if(local_subref->generic_script)
		local_gen_meta = genericscripts[local_subref->generic_script]->meta;
	else local_gen_meta.zero();
	
	for(int q = 0; q < 8; ++q)
	{
		std::string lbl = local_gen_meta.initd[q];
		if(lbl.empty())
			lbl = "InitD["+std::to_string(q)+"]";
		geninitd_lbl[q]->setText(lbl);
		geninitd_btn[q]->setDisabled(local_gen_meta.initd_help[q].empty());
	}
}
void SubscrPropDialog::updateAttr()
{
	switch(local_subref->getType())
	{
		case widgITEMSLOT:
		{
			SW_ItemSlot* w = dynamic_cast<SW_ItemSlot*>(local_subref);
			ddl->setDisabled(w->iid > -1);
			labels[0]->setDisabled(w->iid > -1);
			break;
		}
	}
}
void SubscrPropDialog::updateColors()
{
	switch(local_subref->getType())
	{
		case widgFRAME:
		case widgMINITILE:
		case widgTILEBLOCK:
		{
			tswatches[0]->setCSet(SubscrColorInfo::get_cset(cs_sel[0]->getC1(), cs_sel[0]->getC2()));
			break;
		}
	}
}
void SubscrPropDialog::updateConditions()
{
	bool req_item_empty = local_subref->req_owned_items.empty(), req_not_item_empty = local_subref->req_unowned_items.empty();
	list_reqitems.clear();
	if(req_item_empty)
		list_reqitems.add("---", -1);
	else for(auto iid : local_subref->req_owned_items)
	{
		list_reqitems.add(GUI::ListItem(list_items_no_none.accessItem(iid)));
	}
	
	list_reqnotitems.clear();
	if(req_not_item_empty)
		list_reqnotitems.add("---", -1);
	else for(auto iid : local_subref->req_unowned_items)
	{
		list_reqnotitems.add(GUI::ListItem(list_items_no_none.accessItem(iid)));
	}
	
	if(req_item_list)
	{
		req_item_list->setListData(list_reqitems);
		
		if(req_item_empty)
			req_item_list->setSelectedValue(-1);
		else req_item_list->setSelectedValue(cond_item_sels[CI_REQ]);
		
		cond_item_sels[CI_REQ] = req_item_list->getSelectedValue();
	}
	if(req_not_item_list)
	{
		req_not_item_list->setListData(list_reqnotitems);
		
		if(req_not_item_empty)
			req_not_item_list->setSelectedValue(-1);
		else req_not_item_list->setSelectedValue(cond_item_sels[CI_REQ_NOT]);
		
		cond_item_sels[CI_REQ_NOT] = req_not_item_list->getSelectedValue();
	}
	pendDraw();
}

void SubscrPropDialog::update_wh()
{
	switch(local_subref->getType())
	{
		case widgTILEBLOCK:
			tswatches[0]->setTileWid(std::min(local_subref->w, (word)TB_LA));
			tswatches[0]->setTileHei(std::min(local_subref->h, (word)TB_LA));
			break;
	}
}

void SubscrPropDialog::refr_info()
{
	switch(local_subref->getType())
	{
		case widgITEMSLOT:
		{
			bool dis = get_qr(qr_OLD_SUBSCR);
			def_eqp_cboxes[0]->setDisabled(dis || !get_qr(qr_SELECTAWPN));
			def_eqp_cboxes[1]->setDisabled(dis);
			def_eqp_cboxes[2]->setDisabled(dis || !get_qr(qr_SET_XBUTTON_ITEMS));
			def_eqp_cboxes[3]->setDisabled(dis || !get_qr(qr_SET_YBUTTON_ITEMS));
			break;
		}
		case widgLGAUGE:
		case widgMGAUGE:
		case widgMISCGAUGE:
		{
			SW_GaugePiece* w = dynamic_cast<SW_GaugePiece*>(local_subref);
			bool frcond = w->frames <= 1;
			bool acond = frcond || !(w->flags & (SUBSCR_GAUGE_ANIM_UNDER|SUBSCR_GAUGE_ANIM_OVER));
			bool frcond2 = frcond || (!acond && w->frames <= 2 && (w->flags & SUBSCR_GAUGE_ANIM_SKIP));
			bool infcond = !(w->flags & (SUBSCR_GAUGE_INFITM_REQ|SUBSCR_GAUGE_INFITM_BAN));
			for(int q = 0; q < 8; ++q)
				gauge_gw[q]->setDisabled(!(w->gauge_wid || w->gauge_hei));
			for(int q = 0; q < 4; ++q)
				gauge_tswatches[q]->setIsMini(!(local_subref->flags & SUBSCR_GAUGE_FULLTILE));
			tfs[0]->setDisabled(acond);
			labels[0]->setDisabled(acond);
			tfs[1]->setDisabled(frcond2);
			labels[1]->setDisabled(frcond2);
			tfs[2]->setDisabled(frcond2);
			labels[2]->setDisabled(frcond2);
			ddl->setDisabled(infcond);
			labels[3]->setDisabled(infcond);
			cbs[0]->setDisabled(frcond);
			cbs[1]->setDisabled(frcond);
			cbs[2]->setDisabled(acond);
			cbs[3]->setDisabled(acond);
			break;
		}
	}	
}
bool SubscrPropDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::REFR_SELECTABLE:
			updateSelectable();
			break;
		case message::REFR_INFO:
			refr_info();
			break;
		case message::OK:
			subref->copy_prop(local_subref,true);
			if(set_default_btnslot != start_default_btnslot) //Changed default equipment
			{
				byte pg = subscr_edit.curpage, ind = index;
				for(int q = 0; q < 4; ++q)
				{
					if((start_default_btnslot&btn_flags[q])
						&& !(set_default_btnslot&btn_flags[q]))
					{ //Unset it entirely
						subscr_edit.def_btns[q] = 255;
					}
					else if(!(start_default_btnslot&btn_flags[q])
						&& (set_default_btnslot&btn_flags[q]))
					{ //Set this to be the default
						subscr_edit.def_btns[q] = pg | (byte(subref->pos)<<8);
					}
					else if((start_default_btnslot&btn_flags[q])
						&& byte(subref->pos)!=subscr_edit.def_btns[q]>>8)
					{ //Update the 'pos' value
						subscr_edit.def_btns[q] = pg | (byte(subref->pos)<<8);
					}
				}
			}
			dlg_retval = true;
			return true;
		case message::CANCEL:
			dlg_retval = false;
			return true;
	}
	return false;
}

