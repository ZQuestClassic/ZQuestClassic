#include "subscr_props.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>
#include "zq/zq_subscr.h"
#include "zc_list_data.h"

extern miscQdata misc;
#define QMisc misc

SubscrPropDialog::SubscrPropDialog(subscreen_object *ref, int32_t obj_ind) :
	local_subref(*ref), subref(ref), index(obj_ind),
	list_font(GUI::ZCListData::fonts()),
	list_shadtype(GUI::ZCListData::shadow_types()),
	list_aligns(GUI::ZCListData::alignments()),
	list_buttons(GUI::ZCListData::buttons()),
	list_items(GUI::ZCListData::items(true)),
	list_counters(GUI::ZCListData::counters(true)),
	list_itemclass(GUI::ZCListData::itemclass(true))
{}

static const GUI::ListData two_three_rows
{
	{ "Two", 0 },
	{ "Three", 1 }
}

#define NUM_FIELD(member,_min,_max) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = local_subref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_subref.member = val; \
	})

#define NUM_FIELD2(member,sval,_min,_max) \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	low = _min, high = _max, val = sval, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_subref.member = val; \
	})

#define MISC_COLOR_SEL(txt, num) \
Frame( \
	title = txt, \
	col_sel[num-1] = MiscColorSel( \
		c1 = local_subref.colortype##num, \
		c2 = local_subref.color##num, \
		onUpdate = [&](int32_t c1, int32_t c2) \
		{ \
			local_subref.colortype##num = c1; \
			local_subref.color##num = c2; \
			updateColors(); \
		}) \
)

#define MISC_CSET_SEL(txt, num) \
Frame( \
	title = txt, \
	cs_sel[num-1] = MiscCSetSel( \
		c1 = local_subref.colortype##num, \
		c2 = local_subref.color##num, \
		onUpdate = [&](int32_t c1, int32_t c2) \
		{ \
			local_subref.colortype##num = c1; \
			local_subref.color##num = c2; \
			updateColors(); \
		}) \
)

#define CBOX(member, bit, txt, cspan) \
Checkbox( \
	colSpan = cspan, \
	text = txt, hAlign = 0.0, \
	checked = local_subref.member & bit, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_subref.member, bit, state); \
	} \
)

#define DDL(member, lister) \
DropDownList(data = lister, \
	fitParent = true, \
	selectedValue = local_subref.member, \
	onSelectFunc = [&](int32_t val) \
	{ \
		local_subref.member = val; \
	} \
)
#define DDL_MW(member, lister, maxwid) \
DropDownList(data = lister, \
	fitParent = true, \
	maxwidth = maxwid, \
	selectedValue = local_subref.member, \
	onSelectFunc = [&](int32_t val) \
	{ \
		local_subref.member = val; \
	} \
)

#define GAUGE_MINITILE(txt,mem1,mem2,mem3,bit) \
Frame(fitParent = true, Column(fitParent = true, \
	Label(/*useFont = spfont, */text = txt), \
	SelTileSwatch( \
		hAlign = 0.0, \
		tile = local_subref.mem1>>2, \
		cset = local_subref.mem2, \
		mini = true, \
		minicorner = local_subref.mem1 %4, \
		showvals = false, \
		onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t crn) \
		{ \
			local_subref.mem1 = t<<2; \
			local_subref.mem2 = c; \
			local_subref.mem1 |= crn; \
		} \
	), \
	CBOX(mem3,bit,"Mod",1) \
))

static size_t sprop_tab = 0;
std::shared_ptr<GUI::Widget> SubscrPropDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	char titlebuf[512];
	sprintf(titlebuf, "%s Properties (Object #%d)", sso_name(local_subref.type), index);
	zprint2("Opening: '%s'\n", titlebuf);
	//Generate the basic window, with a reference to the row
	std::shared_ptr<GUI::Grid> windowRow;
	window = Window(
		title = titlebuf,
		onEnter = message::OK,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		Column(
			windowRow = Row(padding = 0_px),
			Row(
				Button(
					text = "&OK",
					topPadding = 0.5_em,
					minwidth = 90_lpx,
					onClick = message::OK,
					focused = true),
				Button(
					text = "&Cancel",
					topPadding = 0.5_em,
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
	
	std::shared_ptr<GUI::Grid> loc_grid;
	//Generate 'location' grid
	{
		//Generate any type-specific location data (ex special w/h things)
		word loadw = sso_w(&local_subref);
		word loadh = sso_h(&local_subref);
		bool show_xy = true;
		bool show_wh = true;
		switch(local_subref.type)
		{
			case sso2X2FRAME:
			case ssoCURRENTITEM:
			case ssoTRIFORCE:
			case ssoTILEBLOCK:
			case ssoMINITILE:
			case ssoTEXTBOX:
				loadw = local_subref.w;
				loadh = local_subref.h;
				break;
			case ssoMAGICMETER:
			case ssoLIFEMETER:
			case ssoMINIMAP:
			case ssoLARGEMAP:
			case ssoSELECTOR1:
			case ssoSELECTOR2:
				show_wh = false;
				break;
			case ssoCLEAR:
				show_xy = false;
				show_wh = false;
				break;
			default: break;
		}
		std::shared_ptr<GUI::Grid> g1;
		loc_grid = Row(
				Column(
					Label(text = "Display:"),
					Checkbox(
						text = "Active Up", hAlign = 0.0,
						checked = local_subref.pos & sspUP,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.pos,sspUP,state);
						}
					),
					Checkbox(
						text = "Active Down", hAlign = 0.0,
						checked = local_subref.pos & sspDOWN,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.pos,sspDOWN,state);
						}
					),
					Checkbox(
						text = "Active Scrolling", hAlign = 0.0,
						checked = local_subref.pos & sspSCROLLING,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.pos,sspSCROLLING,state);
						}
					)
				),
				g1 = Rows<2>()
			);
		if(show_xy)
		{
			g1->add(Label(text = "X:"));
			g1->add(NUM_FIELD(x,-999,9999));
			g1->add(Label(text = "Y:"));
			g1->add(NUM_FIELD(y,-999,9999));
		}
		if(show_wh)
		{
			g1->add(Label(text = "W:"));
			g1->add(NUM_FIELD2(w,loadw,0,999));
			g1->add(Label(text = "H:"));
			g1->add(NUM_FIELD2(h,loadh,0,999));
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
		switch(local_subref.type)
		{
			case sso2X2FRAME:
			{
				col_grid->add(MISC_CSET_SEL("CSet", 1));
				break;
			}
			case ssoBSTIME:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoBUTTONITEM:
				addcolor = false;
				break;
			case ssoCOUNTER:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoCOUNTERS:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoCURRENTITEM:
				addcolor = false;
				break;
			case ssoCLEAR:
			{
				col_grid->add(MISC_COLOR_SEL("Subscreen Color", 1));
				break;
			}
			case ssoTIME:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoSSTIME:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoLARGEMAP:
			{
				col_grid->add(MISC_COLOR_SEL("Room Color", 1));
				col_grid->add(MISC_COLOR_SEL("Player Color", 2));
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
				col_grid->add(MISC_COLOR_SEL("Line Color", 1));
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
				col_grid->add(MISC_COLOR_SEL("Player Color", 1));
				col_grid->add(MISC_COLOR_SEL("Compass Blink Color", 2));
				col_grid->add(MISC_COLOR_SEL("Compass Const Color", 3));
				break;
			}
			case ssoMINIMAPTITLE:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoMINITILE:
			{
				col_grid->add(MISC_CSET_SEL("CSet", 1));
				break;
			}
			case ssoRECT:
			{
				col_grid->add(MISC_COLOR_SEL("Outline Color", 1));
				col_grid->add(MISC_COLOR_SEL("Fill Color", 2));
				break;
			}
			case ssoSELECTEDITEMNAME:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoSELECTOR1:
			{
				col_grid->add(MISC_CSET_SEL("CSet", 1));
				break;
			}
			case ssoSELECTOR2:
			{
				col_grid->add(MISC_CSET_SEL("CSet", 1));
				break;
			}
			case ssoTEXT:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoTEXTBOX:
			{
				col_grid->add(MISC_COLOR_SEL("Text Color", 1));
				col_grid->add(MISC_COLOR_SEL("Shadow Color", 2));
				col_grid->add(MISC_COLOR_SEL("Background Color", 3));
				break;
			}
			case ssoTILEBLOCK:
			{
				col_grid->add(MISC_CSET_SEL("CSet", 1));
				break;
			}
			case ssoTRIFRAME:
			{
				col_grid->add(MISC_COLOR_SEL("Frame Outline Color", 1));
				col_grid->add(MISC_COLOR_SEL("Number Color", 2));
				break;
			}
			case ssoTRIFORCE:
			{
				col_grid->add(MISC_CSET_SEL("CSet", 1));
				break;
			}
		}
	}
	
	std::shared_ptr<GUI::Grid> attrib_grid;
	bool addattrib = true;
	bool force_tabs = false;
	//Generate 'attributes' grid
	{
		switch(local_subref.type)
		{
			case sso2X2FRAME:
			{
				attrib_grid = Column(
					tswatch = SelTileSwatch(
						hAlign = 0.0,
						tile = local_subref.d1,
						cset = subscreen_cset(&QMisc,cs_sel[0]->getC1(), cs_sel[0]->getC2()),
						//flip = local_subref.d2,
						tilewid = 2, tilehei = 2,
						showvals = false,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t)
						{
							local_subref.d1 = t;
							if(local_subref.colortype1 != ssctMISC)
							{
								local_subref.colortype1 = c;
								cs_sel[0]->setC1(c);
							}
						}
					),
					Checkbox(
						text = "Overlay", hAlign = 0.0,
						checked = local_subref.d3,
						onToggleFunc = [&](bool state)
						{
							local_subref.d3 = (state?1:0);
						}
					),
					Checkbox(
						text = "Transparent", hAlign = 0.0,
						checked = local_subref.d4,
						onToggleFunc = [&](bool state)
						{
							local_subref.d4 = (state?1:0);
						}
					)
				);
				break;
			}
			case ssoBSTIME:
			{
				attrib_grid = Columns<3>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(d1, list_font),
					DDL(d3, list_shadtype),
					DDL(d2, list_aligns)
				);
				break;
			}
			case ssoBUTTONITEM:
			{
				attrib_grid = Rows<2>(
					Label(text = "Button:", hAlign = 1.0),
					DDL(d1, list_buttons),
					Checkbox(colSpan = 2,
						text = "Transparent", hAlign = 0.0,
						checked = local_subref.d2,
						onToggleFunc = [&](bool state)
						{
							local_subref.d2 = (state?1:0);
						}
					)
				);
				break;
			}
			case ssoCOUNTER:
			{
				force_tabs = true; //too wide to fit!
				attrib_grid = Columns<6>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					Label(text = "Item 1:", hAlign = 1.0),
					Label(text = "Item 2:", hAlign = 1.0),
					Label(text = "Item 3:", hAlign = 1.0),
					DDL(d1, list_font),
					DDL(d3, list_shadtype),
					DDL(d2, list_aligns),
					DDL(d7, list_counters),
					DDL(d8, list_counters),
					DDL(d9, list_counters),
					Label(text = "Digits:", hAlign = 1.0),
					Label(text = "Infinite:", hAlign = 1.0),
					Label(text = "Inf Character:", hAlign = 1.0),
					Checkbox(
						text = "Show Zero", hAlign = 0.0,
						checked = local_subref.d6 & 0b01,
						colSpan = 2,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.d6, 0b01, state);
						}
					),
					Checkbox(
						colSpan = 2,
						text = "Only Selected", hAlign = 0.0,
						checked = local_subref.d6 & 0b10,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.d6, 0b10, state);
						}
					),
					DummyWidget(colSpan = 2),
					NUM_FIELD(d4,0,5),
					DDL_MW(d10,list_items,100_px),
					TextField(maxLength = 1,
						fitParent = true,
						text = std::string(1,(char)local_subref.d5),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string name(str);
							if(name.size())
								local_subref.d5 = name[0];
							else local_subref.d5 = 0;
						})
				);
				break;
			}
			case ssoCOUNTERS:
			{
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL(d1, list_font),
					Label(text = "Style:", hAlign = 1.0),
					DDL(d3, list_shadtype),
					Label(text = "Digits:", hAlign = 1.0),
					NUM_FIELD(d4,0,5),
					Label(text = "Inf Character:", hAlign = 1.0),
					TextField(maxLength = 1,
						fitParent = true,
						text = std::string(1,(char)local_subref.d5),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string name(str);
							if(name.size())
								local_subref.d5 = name[0];
							else local_subref.d5 = 0;
						}),
					Checkbox(
						text = "Use X", hAlign = 0.0,
						checked = local_subref.d2 & 0b1,
						colSpan = 2,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.d2, 0b1, state);
						}
					)
				);
				break;
			}
			case ssoCURRENTITEM:
			{
				attrib_grid = Rows<2>(
					labels[0] = Label(text = "Item Class:", hAlign = 1.0),
					ddl = DDL(d1, list_itemclass),
					Label(text = "Item Override:", hAlign = 1.0),
					DropDownList(data = list_items,
						fitParent = true,
						forceFitW = true,
						selectedValue = local_subref.d8-1,
						onSelectFunc = [&](int32_t val)
						{
							local_subref.d8 = val+1;
							ddl->setDisabled(val > -1);
							labels[0]->setDisabled(val > -1);
						}
					),
					Label(text = "Position:", hAlign = 1.0),
					NUM_FIELD(d3, -9999, 9999),
					Label(text = "Up Select:", hAlign = 1.0),
					NUM_FIELD(d4, -9999, 9999),
					Label(text = "Down Select:", hAlign = 1.0),
					NUM_FIELD(d5, -9999, 9999),
					Label(text = "Left Select:", hAlign = 1.0),
					NUM_FIELD(d6, -9999, 9999),
					Label(text = "Right Select:", hAlign = 1.0),
					NUM_FIELD(d7, -9999, 9999),
					DummyWidget(),
					Checkbox(
						text = "Invisible", hAlign = 0.0,
						checked = local_subref.d2 & 0b1,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_subref.d2, 0b1, state);
						}
					)
				);
				break;
			}
			case ssoCLEAR:
			{
				addattrib = false;
				break;
			}
			case ssoTIME:
			{
				attrib_grid = Columns<3>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(d1, list_font),
					DDL(d3, list_shadtype),
					DDL(d2, list_aligns)
				);
				break;
			}
			case ssoSSTIME:
			{
				attrib_grid = Columns<3>(
					Label(text = "Font:", hAlign = 1.0),
					Label(text = "Style:", hAlign = 1.0),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(d1, list_font),
					DDL(d3, list_shadtype),
					DDL(d2, list_aligns)
				);
				break;
			}
			case ssoLARGEMAP:
			{
				attrib_grid = Column(
					CBOX(d1, 0b1, "Show Map", 1),
					CBOX(d2, 0b1, "Show Rooms", 1),
					CBOX(d3, 0b1, "Show Hero", 1),
					CBOX(d10, 0b1, "Large", 1)
				);
				break;
			}
			case ssoLIFEGAUGE:
			{
				attrib_grid = Row(padding = 0_px,
					Rows<2>(
						GAUGE_MINITILE("Not Last",d2,colortype1,d10,0x01),
						GAUGE_MINITILE("Last",d3,color1,d10,0x02),
						GAUGE_MINITILE("Cap",d4,colortype2,d10,0x04),
						GAUGE_MINITILE("After Cap",d5,color2,d10,0x08)
					),
					Columns<5>(
						Label(text = "Frames:", hAlign = 1.0),
						Label(text = "Speed:", hAlign = 1.0),
						Label(text = "Delay:", hAlign = 1.0),
						Label(text = "Container:", hAlign = 1.0),
						CBOX(d10,0x10,"Unique Last",2),
						NUM_FIELD(d6, 0, 999),
						NUM_FIELD(d7, 0, 999),
						NUM_FIELD(d8, 0, 999),
						NUM_FIELD(d1, 0, 9999),
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
				attrib_grid = Rows<2>(
					Label(text = "Rows:", hAlign = 1.0),
					DDL(d3, two_three_rows),
					CBOX(d2, 0b1, "Start at bottom", 2)
				);
				break;
			}
			case ssoLINE:
			{
				attrib_grid = Column(
					CBOX(d1, 0b1, "Overlay", 1),
					CBOX(d2, 0b1, "Transparent", 1)
				);
				break;
			}
			case ssoMAGICGAUGE:
			{
				attrib_grid = Row(padding = 0_px,
					Rows<2>(
						GAUGE_MINITILE("Not Last",d2,colortype1,d10,0x01),
						GAUGE_MINITILE("Last",d3,color1,d10,0x02),
						GAUGE_MINITILE("Cap",d4,colortype2,d10,0x04),
						GAUGE_MINITILE("After Cap",d5,color2,d10,0x08)
					),
					Columns<6>(
						Label(text = "Frames:", hAlign = 1.0),
						Label(text = "Speed:", hAlign = 1.0),
						Label(text = "Delay:", hAlign = 1.0),
						Label(text = "Container:", hAlign = 1.0),
						Label(text = "Show:", hAlign = 1.0),
						CBOX(d10,0x10,"Unique Last",2),
						NUM_FIELD(d6, 0, 999),
						NUM_FIELD(d7, 0, 999),
						NUM_FIELD(d8, 0, 999),
						NUM_FIELD(d1, 0, 9999),
						NUM_FIELD(d9, -1, 9999),
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
				attrib_grid = Column(
					CBOX(d1, 0b1, "Show Map", 1),
					CBOX(d2, 0b1, "Show Player", 1),
					CBOX(d3, 0b1, "Show Compass", 1)
				);
				break;
			}
			case ssoMINIMAPTITLE:
			{
				attrib_grid = Rows<2>(
					Label(text = "Font:", hAlign = 1.0),
					DDL(d1, list_font),
					Label(text = "Style:", hAlign = 1.0),
					DDL(d3, list_shadtype),
					Label(text = "Alignment:", hAlign = 1.0),
					DDL(d2, list_aligns)
					CBOX(d4, 0b1, "Invisible w/o Map item", 2)
				);
				break;
			}
			/*case ssoMINITILE:
			{
				attrib_grid = Rows<2>(
					Label(text = txt),
					SelTileSwatch(
						hAlign = 0.0,
						tile = local_subref.mem1>>2,
						cset = local_subref.mem2,
						mini = true,
						minicorner = local_subref.mem1 %4,
						showvals = false,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t crn)
						{
							local_subref.mem1 = t<<2;
							local_subref.mem2 = c;
							local_subref.mem1 |= crn;
						}
					)
				);
				break;
			}*/
			// case ssoRECT:
			// case ssoSELECTEDITEMNAME:
			// case ssoSELECTOR1:
			// case ssoSELECTOR2:
			// case ssoTEXT:
			// case ssoTEXTBOX:
			// case ssoTILEBLOCK:
			// case ssoTRIFRAME:
			// case ssoTRIFORCE:
			default: attrib_grid = Column(Label(text = "WIP!!!")); break;
		}
	}
	if(is_large && !force_tabs)
	{
		windowRow->add(Frame(title = "Location", fitParent = true, loc_grid));
		if(addcolor)
			windowRow->add(Frame(title = "Color", fitParent = true, col_grid));
		if(addattrib)
			windowRow->add(Frame(title = "Attributes", fitParent = true, attrib_grid));
	}
	else
	{
		std::shared_ptr<GUI::TabPanel> tp;
		windowRow->add(
			tp = TabPanel(
				ptr = &sprop_tab
			));
		tp->add(TabRef(name = "Location", loc_grid));
		if(addcolor) tp->add(TabRef(name = "Color", col_grid));
		if(addattrib) tp->add(TabRef(name = "Attributes", attrib_grid));
	}
	return window;
}

void SubscrPropDialog::updateColors()
{
	switch(local_subref.type)
	{
		case sso2X2FRAME:
		{
			tswatch->setCSet(subscreen_cset(&QMisc,cs_sel[0]->getC1(), cs_sel[0]->getC2()));
			break;
		}
	}
}

void save_sso(subscreen_object const& src, subscreen_object* dest)
{
	memcpy(dest, &src, sizeof(subscreen_object));
}

bool SubscrPropDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			save_sso(local_subref, subref);
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

