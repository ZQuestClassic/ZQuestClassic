#include "subscr_props.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>
#include "zq/zq_subscr.h"

SubscrPropDialog::SubscrPropDialog(subscreen_object *ref, int32_t obj_ind) :
	local_subref(*ref), subref(ref), index(obj_ind)
{}

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

#define MISC_COLOR_SEL(txt) \
Frame( \
	title = txt, \
	Label(text = "TODO: Implement Misc Color Selector") \
)

#define MISC_CSET_SEL(txt, mem1, mem2) \
Frame( \
	title = txt, \
	MiscCSetSel( \
		c1 = (local_subref.mem1==ssctMISC ? 0 : local_subref.mem1+1), \
		c2 = local_subref.mem2, \
		onUpdate = [&](int32_t c1, int32_t c2) \
		{ \
			if(c1) \
				local_subref.mem1 = c1-1; \
			else \
				local_subref.mem1 = ssctMISC; \
			local_subref.mem2 = c2; \
		}) \
)

static size_t sprop_tab = 0;
std::shared_ptr<GUI::Widget> SubscrPropDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	char titlebuf[512];
	sprintf(titlebuf, "%s Properties (Object #%d)", sso_name(local_subref.type), index);
	zprint2("Opening: '%s'\n", titlebuf);
	//Generate the basic window, with a reference to the tabpanel to add more objects later
	std::shared_ptr<GUI::TabPanel> windowTP;
	window = Window(
		title = titlebuf,
		onEnter = message::OK,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		Column(
			windowTP = TabPanel(
				ptr = &sprop_tab
			),
			Row(
				Button(
					text = "&OK",
					topPadding = 0.5_em,
					onClick = message::OK,
					focused = true),
				Button(
					text = "&Cancel",
					topPadding = 0.5_em,
					onClick = message::CANCEL)
			)
		)
	);
	
	//Generate location tabref
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
		std::shared_ptr<GUI::TabRef> tr = TabRef(name = "Location", Row(
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
		));
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
		windowTP->add(tr);
	}
	
	//Generate 'color' tabref
	{
		std::shared_ptr<GUI::Grid> g2;
		std::shared_ptr<GUI::TabRef> tr = TabRef(name = "Color", g2 = Column());
		bool addcolor = true;
		switch(local_subref.type)
		{
			case sso2X2FRAME:
			{
				g2->add(MISC_CSET_SEL("CSet", colortype1, color1));
				break;
			}
			case ssoBSTIME:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoBUTTONITEM:
				addcolor = false;
				break;
			case ssoCOUNTER:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoCOUNTERS:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoCURRENTITEM:
				addcolor = false;
				break;
			case ssoCLEAR:
			{
				g2->add(MISC_COLOR_SEL("Subscreen Color"));
				break;
			}
			case ssoTIME:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoSSTIME:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoLARGEMAP:
			{
				g2->add(MISC_COLOR_SEL("Room Color"));
				g2->add(MISC_COLOR_SEL("Player Color"));
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
				g2->add(MISC_COLOR_SEL("Line Color"));
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
				g2->add(MISC_COLOR_SEL("Player Color"));
				g2->add(MISC_COLOR_SEL("Compass Blink Color"));
				g2->add(MISC_COLOR_SEL("Compass Const Color"));
				break;
			}
			case ssoMINIMAPTITLE:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoMINITILE:
			{
				g2->add(MISC_CSET_SEL("CSet", colortype1, color1));
				break;
			}
			case ssoRECT:
			{
				g2->add(MISC_COLOR_SEL("Outline Color"));
				g2->add(MISC_COLOR_SEL("Fill Color"));
				break;
			}
			case ssoSELECTEDITEMNAME:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoSELECTOR1:
			{
				g2->add(MISC_CSET_SEL("CSet", colortype1, color1));
				break;
			}
			case ssoSELECTOR2:
			{
				g2->add(MISC_CSET_SEL("CSet", colortype1, color1));
				break;
			}
			case ssoTEXT:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoTEXTBOX:
			{
				g2->add(MISC_COLOR_SEL("Text Color"));
				g2->add(MISC_COLOR_SEL("Shadow Color"));
				g2->add(MISC_COLOR_SEL("Background Color"));
				break;
			}
			case ssoTILEBLOCK:
			{
				g2->add(MISC_CSET_SEL("CSet", colortype1, color1));
				break;
			}
			case ssoTRIFRAME:
			{
				g2->add(MISC_COLOR_SEL("Frame Outline Color"));
				g2->add(MISC_COLOR_SEL("Number Color"));
				break;
			}
			case ssoTRIFORCE:
			{
				g2->add(MISC_CSET_SEL("CSet", colortype1, color1));
				break;
			}
		}
		if(addcolor) windowTP->add(tr);
	}
	
	switch(local_subref.type)
	{
		default: break;
	}
	return window;
}

void save_sso(subscreen_object const& src, subscreen_object* dest)
{
	switch(src.type)
	{
		default:
			InfoDialog("WIP","Changes could not be saved for this type, WIP error").show();
			break;
	}
}

bool SubscrPropDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			//save_sso(local_subref, subref);
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

