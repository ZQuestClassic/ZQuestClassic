#include "subscr_props.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>
#include <sstream>
#include <fmt/format.h>
#include "zq/zq_subscr.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"
#include "base/misctypes.h"

extern script_data *genericscripts[NUMSCRIPTSGENERIC];
extern ZCSubscreen subscr_edit;

static bool dlg_retval = false;
bool call_subscrprop_dialog(SubscrWidget* widg, int32_t obj_ind)
{
	SubscrPropDialog(widg,obj_ind).show();
	return dlg_retval;
}

static const int btn_flags[4] = {INT_BTN_A,INT_BTN_B,INT_BTN_X,INT_BTN_Y};
SubscrPropDialog::SubscrPropDialog(SubscrWidget* widg, int32_t obj_ind) :
	local_subref(widg->clone()), subref(widg), index(obj_ind),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_shadtype(GUI::ZCListData::shadow_types()),
	list_aligns(GUI::ZCListData::alignments()),
	list_buttons(GUI::ZCListData::buttons()),
	list_items(GUI::ZCListData::items(true)),
	list_counters(GUI::ZCListData::ss_counters()),
	list_itemclass(GUI::ZCListData::itemclass(true)),
	list_genscr(GUI::ZCListData::generic_script())
{
	byte pg = subscr_edit.curpage, ind = index;
	start_default_btnslot = 0;
	if(widg->getType() == widgITEMSLOT)
	{
		for(int q = 0; q < 4; ++q)
		{
			if((subscr_edit.def_btns[q]&0xFF) == pg
				&& (subscr_edit.def_btns[q]>>8) == ind)
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

#define NUM_FIELD(var,_min,_max) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = var, \
	onValChangedFunc = [=](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		var = val; \
	})

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

#define CBOX(var, bit, txt, cspan) \
Checkbox( \
	colSpan = cspan, \
	text = txt, hAlign = 0.0, \
	checked = var & bit, \
	onToggleFunc = [=](bool state) \
	{ \
		SETFLAG(var, bit, state); \
	} \
)

#define _EX_RBOX hAlign = 1.0,boxPlacement = GUI::Checkbox::boxPlacement::RIGHT
#define CBOX_EX(var, bit, txt, ...) \
Checkbox( \
	__VA_ARGS__, \
	text = txt, \
	checked = var & bit, \
	onToggleFunc = [=](bool state) \
	{ \
		SETFLAG(var, bit, state); \
	} \
)

#define DDL(var, lister) \
DropDownList(data = lister, \
	fitParent = true, \
	selectedValue = var, \
	onSelectFunc = [=](int32_t val) \
	{ \
		var = val; \
	} \
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
#define DDL_MW(var, lister, maxwid) \
DropDownList(data = lister, \
	fitParent = true, \
	maxwidth = maxwid, \
	selectedValue = var, \
	onSelectFunc = [=](int32_t val) \
	{ \
		var = val; \
	} \
)

#define GAUGE_MINITILE(txt,vMTInfo,vModflag,bit) \
Frame(fitParent = true, Column(fitParent = true, \
	Label(/*useFont = spfont, */text = txt), \
	SelTileSwatch( \
		hAlign = 0.0, \
		tile = vMTInfo.tile(), \
		cset = vMTInfo.cset, \
		mini = true, \
		minicorner = vMTInfo.crn(), \
		showvals = false, \
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

static size_t sprop_tab = 0;
static char tbuf[1025] = {0};
std::shared_ptr<GUI::Widget> SubscrPropDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	char titlebuf[512];
	sprintf(titlebuf, "%s Properties (Object #%d)", sso_name(local_subref->getType()), index);
	
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
			case widgMMETER:
			case widgLMETER:
			case widgMMAP:
			case widgLMAP:
			case widgSELECTOR:
			case widgMINITILE:
			case widgMCGUFF_FRAME:
				show_wh = false;
				break;
			case widgBGCOLOR:
				show_xy = false;
				show_wh = false;
				break;
			default: break;
		}
		std::shared_ptr<GUI::Grid> g1;
		loc_grid = Column(
				Row(
					Column(
						Label(text = "Display:"),
						Checkbox(
							text = "Active Up", hAlign = 0.0,
							checked = local_subref->posflags & sspUP,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspUP,state);
							}
						),
						Checkbox(
							text = "Active Down", hAlign = 0.0,
							checked = local_subref->posflags & sspDOWN,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspDOWN,state);
							}
						),
						Checkbox(
							text = "Active Scrolling", hAlign = 0.0,
							checked = local_subref->posflags & sspSCROLLING,
							onToggleFunc = [=](bool state)
							{
								SETFLAG(local_subref->posflags,sspSCROLLING,state);
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
	
	std::shared_ptr<GUI::Grid> col_grid = Column();
	bool addcolor = true;
	//Generate 'color' grid
	{
		switch(local_subref->getType())
		{
			case widgFRAME:
			{
				SW_2x2Frame* w = dynamic_cast<SW_2x2Frame*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgTIME:
			{
				SW_Time* w = dynamic_cast<SW_Time*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgBTNITM:
				addcolor = false;
				break;
			case widgCOUNTER:
			{
				SW_Counter* w = dynamic_cast<SW_Counter*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgOLDCTR:
			{
				SW_Counters* w = dynamic_cast<SW_Counters*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgITEMSLOT:
				addcolor = false;
				break;
			case widgBGCOLOR:
			{
				SW_Clear* w = dynamic_cast<SW_Clear*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Subscreen Color", 1));
				break;
			}
			case widgLMAP:
			{
				SW_LMap* w = dynamic_cast<SW_LMap*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_room, "Room Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_plr, "Player Color", 2));
				break;
			}
			case widgLGAUGE:
				addcolor = false;
				break;
			case widgLMETER:
				addcolor = false;
				break;
			case widgLINE:
			{
				SW_Line* w = dynamic_cast<SW_Line*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_line, "Line Color", 1));
				break;
			}
			case widgMGAUGE:
				addcolor = false;
				break;
			case widgMMETER:
				addcolor = false;
				break;
			case widgMMAP:
			{
				SW_MMap* w = dynamic_cast<SW_MMap*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_plr, "Player Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_cmp_blink, "Compass Blink Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_cmp_off, "Compass Const Color", 3));
				break;
			}
			case widgMMAPTITLE:
			{
				SW_MMapTitle* w = dynamic_cast<SW_MMapTitle*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgMINITILE:
			{
				SW_MiniTile* w = dynamic_cast<SW_MiniTile*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgRECT:
			{
				SW_Rect* w = dynamic_cast<SW_Rect*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_outline, "Outline Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_fill, "Fill Color", 2));
				break;
			}
			case widgSELECTEDTEXT:
			{
				SW_SelectedText* w = dynamic_cast<SW_SelectedText*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgSELECTOR:
				addcolor = false;
				break;
			case widgTEXT:
			{
				SW_Text* w = dynamic_cast<SW_Text*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgTEXTBOX:
			{
				SW_TextBox* w = dynamic_cast<SW_TextBox*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case widgTILEBLOCK:
			{
				SW_TileBlock* w = dynamic_cast<SW_TileBlock*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case widgMCGUFF_FRAME:
			{
				SW_TriFrame* w = dynamic_cast<SW_TriFrame*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_outline, "Frame Outline Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_number, "Number Color", 2));
				break;
			}
			case widgMCGUFF:
			{
				SW_McGuffin* w = dynamic_cast<SW_McGuffin*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
		}
	}
	
	std::shared_ptr<GUI::Grid> attrib_grid;
	bool addattrib = true;
	enum { mtNONE, mtFORCE_TAB, mtLOCTOP };
	int32_t mergetype = mtNONE;
	//Generate 'attributes' grid
	{
		switch(local_subref->getType())
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
				attrib_grid = Columns<6>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					Label(text = "Item 1:", hAlign = 1.0),
					Label(text = "Item 2:", hAlign = 1.0),
					Label(text = "Item 3:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					DDL(w->shadtype, list_shadtype),
					DDL(w->align, list_aligns),
					DDL(w->ctrs[0], list_counters),
					DDL(w->ctrs[1], list_counters),
					DDL(w->ctrs[2], list_counters),
					Label(text = "Digits:", hAlign = 1.0),
					Label(text = "Infinite:", hAlign = 1.0),
					Label(text = "Inf Character:", hAlign = 1.0),
					CBOX(w->flags,SUBSCR_COUNTER_SHOW0,"Show Zero",2),
					CBOX(w->flags,SUBSCR_COUNTER_ONLYSEL,"Only Selected",2),
					DummyWidget(colSpan = 2),
					NUM_FIELD(w->digits,0,5),
					DDL_MW(w->infitm,list_items,100_px),
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
				);
				break;
			}
			case widgOLDCTR:
			{
				SW_Counters* w = dynamic_cast<SW_Counters*>(local_subref);
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
							ddl->setDisabled(val > -1);
							labels[0]->setDisabled(val > -1);
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
						" displays and changing heart pieces into the heart piece display) will not occur for this widget.")
				);
				break;
			}
			case widgBGCOLOR:
			{
				addattrib = false;
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
			{
				SW_LifeGaugePiece* w = dynamic_cast<SW_LifeGaugePiece*>(local_subref);
				attrib_grid = Row(padding = 0_px,
					Rows<2>(
						GAUGE_MINITILE("Not Last",w->mts[0],w->flags,SUBSCR_LGAUGE_MOD1),
						GAUGE_MINITILE("Last",w->mts[1],w->flags,SUBSCR_LGAUGE_MOD2),
						GAUGE_MINITILE("Cap",w->mts[2],w->flags,SUBSCR_LGAUGE_MOD3),
						GAUGE_MINITILE("After Cap",w->mts[3],w->flags,SUBSCR_LGAUGE_MOD4)
					),
					Columns<5>(
						Label(text = "Frames:", hAlign = 1.0),
						Label(text = "Speed:", hAlign = 1.0),
						Label(text = "Delay:", hAlign = 1.0),
						Label(text = "Container:", hAlign = 1.0),
						CBOX(w->flags,SUBSCR_LGAUGE_UNQLAST,"Unique Last",2),
						NUM_FIELD(w->frames, 1, 999),
						NUM_FIELD(w->speed, 1, 999),
						NUM_FIELD(w->delay, 0, 999),
						NUM_FIELD(w->container, 0, 9999),
						DummyWidget(rowSpan=3),
						INFOBTN("The container number this piece represents. For a value of n,"
							"\nIf the Player has exactly n containers, 'Last' displays."
							"\nIf the Player has > n containers, 'Not Last' displays."
							"\nIf the Player has exactly n-1 containers, 'Cap' displays."
							"\nIf the Player has < n-1 containers, 'After Cap' displays."
							)
					)
				);
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
			case widgMGAUGE:
			{
				SW_MagicGaugePiece* w = dynamic_cast<SW_MagicGaugePiece*>(local_subref);
				attrib_grid = Row(padding = 0_px,
					Rows<2>(
						GAUGE_MINITILE("Not Last",w->mts[0],w->flags,SUBSCR_MGAUGE_MOD1),
						GAUGE_MINITILE("Last",w->mts[1],w->flags,SUBSCR_MGAUGE_MOD2),
						GAUGE_MINITILE("Cap",w->mts[2],w->flags,SUBSCR_MGAUGE_MOD3),
						GAUGE_MINITILE("After Cap",w->mts[3],w->flags,SUBSCR_MGAUGE_MOD4)
					),
					Columns<6>(
						Label(text = "Frames:", hAlign = 1.0),
						Label(text = "Speed:", hAlign = 1.0),
						Label(text = "Delay:", hAlign = 1.0),
						Label(text = "Container:", hAlign = 1.0),
						Label(text = "Show:", hAlign = 1.0),
						CBOX(w->flags,SUBSCR_MGAUGE_UNQLAST,"Unique Last",2),
						NUM_FIELD(w->frames, 1, 999),
						NUM_FIELD(w->speed, 1, 999),
						NUM_FIELD(w->delay, 0, 999),
						NUM_FIELD(w->container, 0, 9999),
						NUM_FIELD(w->showdrain, -1, 9999),
						DummyWidget(rowSpan=3),
						INFOBTN("The container number this piece represents. For a value of n,"
							"\nIf the Player has exactly n containers, 'Last' displays."
							"\nIf the Player has > n containers, 'Not Last' displays."
							"\nIf the Player has exactly n-1 containers, 'Cap' displays."
							"\nIf the Player has < n-1 containers, 'After Cap' displays."
							),
						INFOBTN("If set to -1, piece shows normally."
							"\nIf set to > -1, piece will only be visible when your"
							" Magic Drain Rate is equal to the value."
							"\nEx. With the default of starting at drain rate 2,"
							" a show value of '1' would be used for a '1/2 magic' icon."
							"\nWith a starting drain rate of 4, you could"
							" then have a show value of '2' for a '1/2 magic', and a"
							" show value of '1' for a '1/4 magic'.")
					)
				);
				break;
			}
			case widgMMETER:
			{
				addattrib = false;
				break;
			}
			case widgMMAP:
			{
				SW_MMap* w = dynamic_cast<SW_MMap*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_MMAP_SHOWMAP, "Show Map", 1),
					CBOX(w->flags, SUBSCR_MMAP_SHOWPLR, "Show Player", 1),
					CBOX(w->flags, SUBSCR_MMAP_SHOWCMP, "Show Compass", 1)
				);
				break;
			}
			case widgMMAPTITLE:
			{
				SW_MMapTitle* w = dynamic_cast<SW_MMapTitle*>(local_subref);
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					Label(text = "Style:", hAlign = 1.0),
					DDL(w->shadtype, list_shadtype),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(w->align, list_aligns),
					CBOX(w->flags, SUBSCR_MMAPTIT_REQMAP, "Invisible w/o Map item", 2)
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
						cset = w->cset,
						showvals = false,
						// tilewid = local_subref->d7 ? 2 : 1,
						// tilehei = local_subref->d7 ? 3 : 1,
						deftile = QMisc.colors.triforce_tile,
						defcs = QMisc.colors.triforce_cset,
						onSelectFunc = [=](int32_t t, int32_t c, int32_t,int32_t)
						{
							w->tile = t;
							w->cset = c;
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
			default: attrib_grid = Column(Label(text = "ERROR")); break;
		}
	}
	
	std::shared_ptr<GUI::Grid> g;
	std::shared_ptr<GUI::TabPanel> tpan = TabPanel(ptr = &sprop_tab);
	switch(mergetype)
	{
		default:
		case mtNONE: //3 in horz row
		{
			tpan->add(TabRef(name = "Basic", g = Rows<2>(padding = 0_px)));
			g->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(addcolor)
				g->add(Frame(title = "Color", fitParent = true, col_grid));
			if(addattrib)
				g->add(Frame(title = "Attributes", fitParent = true, attrib_grid));
			break;
		}
		case mtFORCE_TAB: //3 separate tabs
		{
			tpan->add(TabRef(name = "Location", loc_grid));
			if(addcolor) tpan->add(TabRef(name = "Color", col_grid));
			if(addattrib) tpan->add(TabRef(name = "Attributes", attrib_grid));
			break;
		}
		case mtLOCTOP:
		{
			tpan->add(TabRef(name = "Basic", g = Row(padding = 0_px)));
			if(addcolor)
			{
				g->add(Column(padding = 0_px,
					Frame(title = "Location", fitParent = true, loc_grid),
					Frame(title = "Color", fitParent = true, col_grid)));
			}
			else g->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(addattrib)
				g->add(Frame(title = "Attributes", fitParent = true, attrib_grid));
			break;
		}
	}
	tpan->add(TabRef(name = "Selection", Rows<2>(
			Frame(title = "Cursor Selectable", info = "If the subscreen cursor can select this"
				" widget, and the selectable position information for selecting it.",
				fitParent = true,
				Column(padding = 0_px,
					Checkbox(
						text = "Is Selectable",
						checked = local_subref->flags & SUBSCRFLAG_SELECTABLE,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref->flags, SUBSCRFLAG_SELECTABLE, state);
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
			selframes[0] = Frame(title = "Generic Frozen Script",
				info = "Run a Generic Frozen Script when a button is pressed."
					"\nThe script will run before any other effects that occur from"
					" pressing the button (such as equipping an item to a button).",
					fitParent = true,
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
	updateSelectable();
	refr_info();
	return window;
}

void SubscrPropDialog::updateSelectable()
{
	bool seldis = !(local_subref->flags & SUBSCRFLAG_SELECTABLE);
	selgs[0]->setDisabled(seldis);
	selframes[0]->setDisabled(seldis);
	selframes[1]->setDisabled(seldis);
	selgs[1]->setDisabled(seldis || !local_subref->generic_script);
	selgs[2]->setDisabled(seldis || !local_subref->generic_script);
	
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
	}	
}
bool SubscrPropDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
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

