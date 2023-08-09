#include "subscr_props.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>
#include <sstream>
#include "zq/zq_subscr.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"
#include "base/misctypes.h"

static bool dlg_retval = false;
bool call_subscrprop_dialog(SubscrWidget* widg, int32_t obj_ind)
{
	SubscrPropDialog(widg,obj_ind).show();
	return dlg_retval;
}

SubscrPropDialog::SubscrPropDialog(SubscrWidget* widg, int32_t obj_ind) :
	local_subref(widg->clone()), subref(widg), index(obj_ind),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_shadtype(GUI::ZCListData::shadow_types()),
	list_aligns(GUI::ZCListData::alignments()),
	list_buttons(GUI::ZCListData::buttons()),
	list_items(GUI::ZCListData::items(true)),
	list_counters(GUI::ZCListData::ss_counters()),
	list_itemclass(GUI::ZCListData::itemclass(true))
{}

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

static size_t sprop_tab = 0;
static char tbuf[1025] = {0};
std::shared_ptr<GUI::Widget> SubscrPropDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	char titlebuf[512];
	sprintf(titlebuf, "%s Properties (Object #%d)", sso_name(local_subref->getType()), index);
	//Generate the basic window, with a reference to the row
	std::shared_ptr<GUI::Grid> windowRow;
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		Column(
			windowRow = Row(padding = 0_px),
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
			case sso2X2FRAME:
			case ssoCURRENTITEM:
			case ssoMCGUFFIN:
			case ssoTILEBLOCK:
			case ssoTEXTBOX:
				loadw = local_subref->w;
				loadh = local_subref->h;
				break;
			case ssoMAGICMETER:
			case ssoLIFEMETER:
			case ssoMINIMAP:
			case ssoLARGEMAP:
			case ssoSELECTOR1:
			case ssoSELECTOR2:
			case ssoMINITILE:
			case ssoTRIFRAME:
				show_wh = false;
				break;
			case ssoCLEAR:
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
				),
				Rows<3>(
					CBOX(local_subref->flags,SUBSCRFLAG_SELECTABLE,"Cursor Selectable",3),
					//
					Label(text = "Position:", hAlign = 1.0),
					NUM_FIELD(local_subref->pos, -9999, 9999),
					INFOBTN("The unique position ID of this slot"),
					Label(text = "Up Select:", hAlign = 1.0),
					NUM_FIELD(local_subref->pos_up, -9999, 9999),
					INFOBTN("The unique position ID to move to when pressing 'Up'"),
					Label(text = "Down Select:", hAlign = 1.0),
					NUM_FIELD(local_subref->pos_down, -9999, 9999),
					INFOBTN("The unique position ID to move to when pressing 'Down'"),
					Label(text = "Left Select:", hAlign = 1.0),
					NUM_FIELD(local_subref->pos_left, -9999, 9999),
					INFOBTN("The unique position ID to move to when pressing 'Left' / 'L' quickswap"),
					Label(text = "Right Select:", hAlign = 1.0),
					NUM_FIELD(local_subref->pos_right, -9999, 9999),
					INFOBTN("The unique position ID to move to when pressing 'Right' / 'R' quickswap")
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
			case sso2X2FRAME:
			{
				SW_2x2Frame* w = dynamic_cast<SW_2x2Frame*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case ssoBSTIME:
			case ssoTIME:
			case ssoSSTIME:
			{
				SW_Time* w = dynamic_cast<SW_Time*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoBUTTONITEM:
				addcolor = false;
				break;
			case ssoCOUNTER:
			{
				SW_Counter* w = dynamic_cast<SW_Counter*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoCOUNTERS:
			{
				SW_Counters* w = dynamic_cast<SW_Counters*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoCURRENTITEM:
				addcolor = false;
				break;
			case ssoCLEAR:
			{
				SW_Clear* w = dynamic_cast<SW_Clear*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Subscreen Color", 1));
				break;
			}
			case ssoLARGEMAP:
			{
				SW_LMap* w = dynamic_cast<SW_LMap*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_room, "Room Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_plr, "Player Color", 2));
				break;
			}
			case ssoLIFEGAUGE:
				addcolor = false;
				break;
			case ssoLIFEMETER:
				addcolor = false;
				break;
			case ssoLINE:
			{
				SW_Line* w = dynamic_cast<SW_Line*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_line, "Line Color", 1));
				break;
			}
			case ssoMAGICGAUGE:
				addcolor = false;
				break;
			case ssoMAGICMETER:
				addcolor = false;
				break;
			case ssoMINIMAP:
			{
				SW_MMap* w = dynamic_cast<SW_MMap*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_plr, "Player Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_cmp_blink, "Compass Blink Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_cmp_off, "Compass Const Color", 3));
				break;
			}
			case ssoMINIMAPTITLE:
			{
				SW_MMapTitle* w = dynamic_cast<SW_MMapTitle*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoMINITILE:
			{
				SW_MiniTile* w = dynamic_cast<SW_MiniTile*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case ssoRECT:
			{
				SW_Rect* w = dynamic_cast<SW_Rect*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_outline, "Outline Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_fill, "Fill Color", 2));
				break;
			}
			case ssoSELECTEDITEMNAME:
			{
				SW_SelectedText* w = dynamic_cast<SW_SelectedText*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoSELECTOR1:
			case ssoSELECTOR2:
				addcolor = false;
				break;
			case ssoTEXT:
			{
				SW_Text* w = dynamic_cast<SW_Text*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoTEXTBOX:
			{
				SW_TextBox* w = dynamic_cast<SW_TextBox*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_text, "Text Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_shadow, "Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL(w->c_bg, "Background Color", 3));
				break;
			}
			case ssoTILEBLOCK:
			{
				SW_TileBlock* w = dynamic_cast<SW_TileBlock*>(local_subref);
				col_grid->add(MISC_CSET_SEL(w->cs, "CSet", 1));
				break;
			}
			case ssoTRIFRAME:
			{
				SW_TriFrame* w = dynamic_cast<SW_TriFrame*>(local_subref);
				col_grid->add(MISC_COLOR_SEL(w->c_outline, "Frame Outline Color", 1));
				col_grid->add(MISC_COLOR_SEL(w->c_number, "Number Color", 2));
				break;
			}
			case ssoMCGUFFIN:
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
			case sso2X2FRAME:
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
			case ssoTIME:
			case ssoSSTIME:
			case ssoBSTIME:
			{
				SW_Time* w = dynamic_cast<SW_Time*>(local_subref);
				attrib_grid = Columns<3>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL_FONT(w->fontid),
					DDL(w->shadtype, list_shadtype),
					DDL(w->align, list_aligns)
				);
				break;
			}
			case ssoBUTTONITEM:
			{
				SW_ButtonItem* w = dynamic_cast<SW_ButtonItem*>(local_subref);
				attrib_grid = Rows<2>(
					Label(text = "Button:", hAlign = 1.0),
					DDL(w->btn, list_buttons),
					CBOX(w->flags,SUBSCR_BTNITM_TRANSP,"Transparent",2)
				);
				break;
			}
			case ssoCOUNTER:
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
			case ssoCOUNTERS:
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
			case ssoCURRENTITEM:
			{
				SW_CurrentItem* w = dynamic_cast<SW_CurrentItem*>(local_subref);
				attrib_grid = Rows<3>(
					labels[0] = Label(text = "Item Class:", hAlign = 1.0),
					ddl = DDL(w->iclass, list_itemclass),
					INFOBTN("The highest level owned of this itemclass will be tied to this item slot."),
					Label(text = "Item Override:", hAlign = 1.0),
					DropDownList(data = list_items,
						fitParent = true,
						forceFitW = true,
						selectedValue = w->iid-1,
						onSelectFunc = [=](int32_t val)
						{
							w->iid = val+1;
							ddl->setDisabled(val > -1);
							labels[0]->setDisabled(val > -1);
						}
					),
					INFOBTN("The specified item ID will be tied to this item slot (OVERRIDES 'Item Class' if set)"),
					DummyWidget(),
					CBOX(w->flags,SUBSCR_CURITM_INVIS,"Invisible",1),
					INFOBTN("If checked, the item is invisible on the subscreen"),
					DummyWidget(),
					CBOX(w->flags,SUBSCR_CURITM_NONEQP,"Non-Equippable",1),
					INFOBTN("If checked, the item cannot be equipped to a button."
						" 'Always Press To Equip' is recommended if this is used." + QRHINT({qr_SUBSCR_PRESS_TO_EQUIP}))
				);
				break;
			}
			case ssoCLEAR:
			{
				addattrib = false;
				break;
			}
			case ssoLARGEMAP:
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
			case ssoLIFEGAUGE:
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
						NUM_FIELD(w->frames, 0, 999),
						NUM_FIELD(w->speed, 0, 999),
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
			case ssoLIFEMETER:
			{
				SW_LifeMeter* w = dynamic_cast<SW_LifeMeter*>(local_subref);
				attrib_grid = Rows<2>(
					Label(text = "Rows:", hAlign = 1.0),
					DDL(w->rows, two_three_rows),
					CBOX(w->flags, SUBSCR_LIFEMET_BOT, "Start at bottom", 2)
				);
				break;
			}
			case ssoLINE:
			{
				SW_Line* w = dynamic_cast<SW_Line*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_LINE_TRANSP, "Transparent", 1)
				);
				break;
			}
			case ssoMAGICGAUGE:
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
						NUM_FIELD(w->frames, 0, 999),
						NUM_FIELD(w->speed, 0, 999),
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
			case ssoMAGICMETER:
			{
				addattrib = false;
				break;
			}
			case ssoMINIMAP:
			{
				SW_MMap* w = dynamic_cast<SW_MMap*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_MMAP_SHOWMAP, "Show Map", 1),
					CBOX(w->flags, SUBSCR_MMAP_SHOWPLR, "Show Player", 1),
					CBOX(w->flags, SUBSCR_MMAP_SHOWCMP, "Show Compass", 1)
				);
				break;
			}
			case ssoMINIMAPTITLE:
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
			case ssoMINITILE:
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
			case ssoRECT:
			{
				SW_Rect* w = dynamic_cast<SW_Rect*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_RECT_FILLED, "Filled", 1),
					CBOX(w->flags, SUBSCR_RECT_TRANSP, "Transparent", 1)
				);
				break;
			}
			case ssoSELECTEDITEMNAME:
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
			case ssoSELECTOR1:
			case ssoSELECTOR2:
			{
				SW_Selector* w = dynamic_cast<SW_Selector*>(local_subref);
				attrib_grid = Column(
					CBOX(w->flags, SUBSCR_SELECTOR_TRANSP, "Transparent", 1),
					CBOX(w->flags, SUBSCR_SELECTOR_LARGE, "Large", 1),
					CBOX(w->flags, SUBSCR_SELECTOR_USEB, "Use Selector 2", 1)
				);
				break;
			}
			case ssoTEXT:
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
			case ssoTEXTBOX:
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
			case ssoTILEBLOCK:
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
			case ssoTRIFRAME:
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
			case ssoMCGUFFIN:
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
	switch(mergetype)
	{
		default:
		case mtNONE: //3 in horz row
		{
			windowRow->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(addcolor)
				windowRow->add(Frame(title = "Color", fitParent = true, col_grid));
			if(addattrib)
				windowRow->add(Frame(title = "Attributes", fitParent = true, attrib_grid));
			break;
		}
		case mtFORCE_TAB: //3 separate tabs
		{
			std::shared_ptr<GUI::TabPanel> tp;
			windowRow->add(
				tp = TabPanel(
					ptr = &sprop_tab
				));
			tp->add(TabRef(name = "Location", loc_grid));
			if(addcolor) tp->add(TabRef(name = "Color", col_grid));
			if(addattrib) tp->add(TabRef(name = "Attributes", attrib_grid));
			break;
		}
		case mtLOCTOP:
		{
			if(addcolor)
			{
				windowRow->add(Column(padding = 0_px,
					Frame(title = "Location", fitParent = true, loc_grid),
					Frame(title = "Color", fitParent = true, col_grid)));
			}
			else windowRow->add(Frame(title = "Location", fitParent = true, loc_grid));
			if(addattrib)
				windowRow->add(Frame(title = "Attributes", fitParent = true, attrib_grid));
			break;
		}
	}
	return window;
}

void SubscrPropDialog::updateColors()
{
	switch(local_subref->getType())
	{
		case sso2X2FRAME:
		case ssoMINITILE:
		case ssoTILEBLOCK:
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
		case ssoTILEBLOCK:
			tswatches[0]->setTileWid(std::min(local_subref->w, (word)TB_LA));
			tswatches[0]->setTileHei(std::min(local_subref->h, (word)TB_LA));
			break;
	}
}

bool SubscrPropDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			subref->copy_prop(local_subref,true);
			dlg_retval = true;
			return true;
		case message::CANCEL:
			dlg_retval = false;
			return true;
	}
	return false;
}

