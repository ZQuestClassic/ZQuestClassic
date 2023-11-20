#include "zc_options.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "base/fonts.h"
#include "tiles.h"
#include "zc_list_data.h"
#include "alert.h"

extern bool reload_fonts;
void load_size_poses();

void call_zc_options_dlg()
{
	ZCOptionsDialog().show();
}

void ZCOptionsDialog::loadOptions()
{
	char const* _font_titles[CFONT_MAX] = {"dialog", "gui", "title", "favcmd", "textbox", "ttip", "info"};
	for(int q = 0; q < CFONT_MAX; ++q)
	{
		opts[q+OPT_FIRSTFONT] = zc_get_config("ZQ_GUI",
			get_font_cfgname(false, q).c_str(), get_def_fontid(q));
		opts[q+OPT_FIRST_FONTSCALE] = zc_get_config("ZQ_GUI",
			get_font_cfgname(true, q).c_str(), 1);
	}
}

void ZCOptionsDialog::saveOptions()
{
	for(auto ind = 0; ind < OPT_MAX; ++ind)
	{
		if(opt_changed[ind])
			saveOption(ind);
	}
	for(int ind = OPT_FIRSTFONT; ind <= OPT_LAST_FONTSCALE; ++ind)
	{
		if(opt_changed[ind])
		{
			reload_fonts = true;
			break;
		}
	}
	//set_keyboard_rate(KeyboardRepeatDelay,KeyboardRepeatRate); //Reset keyboard rate
	//load_mice(); //Reset cursor scale
}
void ZCOptionsDialog::saveOption(int ind)
{
	auto v = opts[ind];
	if(ind >= OPT_FIRSTFONT && ind <= OPT_LASTFONT)
		zc_set_config("ZQ_GUI", get_font_cfgname(false, ind-OPT_FIRSTFONT).c_str(), v);
	else if(ind >= OPT_FIRST_FONTSCALE && ind <= OPT_LAST_FONTSCALE)
		zc_set_config("ZQ_GUI", get_font_cfgname(true, ind-OPT_FIRSTFONT).c_str(), v);
	// else switch(ind)
	// {
		// default:
			// break;
	// }
}

ZCOptionsDialog::ZCOptionsDialog() : sfx_list(GUI::ZCListData::sfxnames(true))
{
	loadOptions();
	memset(opt_changed, 0, sizeof(opt_changed));
}

//{ Macros
#define OPT_CHECK(optind, optlabel) \
Checkbox( \
	checked = opts[optind], \
	text = optlabel, \
	hAlign = 0.0, \
	onToggleFunc = [&](bool state) \
	{ \
		opts[optind] = state ? 1 : 0; \
		opt_changed[optind] = true; \
	} \
)

#define ROW_CHECK(optind, optlabel) \
Checkbox( \
	colSpan = 2, \
	checked = opts[optind], \
	text = optlabel, \
	hAlign = 0.0, \
	vPadding = DEFAULT_PADDING/2, \
	onToggleFunc = [&](bool state) \
	{ \
		opts[optind] = state ? 1 : 0; \
		opt_changed[optind] = true; \
	} \
), \
Button(forceFitH = true, text = "?", padding = 0_px, disabled = true)

#define ROW_CHECK_I(optind, optlabel, info) \
Checkbox( \
	colSpan = 2, \
	checked = opts[optind], \
	text = optlabel, \
	hAlign = 0.0, \
	vPadding = DEFAULT_PADDING/2, \
	onToggleFunc = [&](bool state) \
	{ \
		opts[optind] = state ? 1 : 0; \
		opt_changed[optind] = true; \
	} \
), \
Button(forceFitH = true, text = "?", \
	disabled = !info[0], padding = 0_px, \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})

#define ROW_TF_RANGE(optind, optlabel, minval, maxval) \
Label(text = optlabel, hAlign = 0.0), \
TextField(type = GUI::TextField::type::INT_DECIMAL, \
	fitParent = true, \
	hAlign = 1.0, low = minval, high = maxval, val = opts[optind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	}), \
DummyWidget()

#define ROW_TF_RANGE_I(optind, optlabel, minval, maxval, info) \
Label(text = optlabel, hAlign = 0.0), \
TextField(type = GUI::TextField::type::INT_DECIMAL, \
	fitParent = true, \
	hAlign = 1.0, low = minval, high = maxval, val = opts[optind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	}), \
Button(forceFitH = true, text = "?", \
	disabled = !info[0], \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})

#define ROW_TF_FLOAT(optind, optlabel, minval, maxval) \
Label(text = optlabel, hAlign = 0.0), \
TextField(type = GUI::TextField::type::FIXED_DECIMAL, places = 4, \
	fitParent = true, maxLength = 8, \
	hAlign = 1.0, low = minval*10000, high = maxval*10000, val = opts[optind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	}), \
DummyWidget()

#define ROW_TF_FLOAT_I(optind, optlabel, minval, maxval, info) \
Label(text = optlabel, hAlign = 0.0), \
TextField(type = GUI::TextField::type::FIXED_DECIMAL, places = 4, \
	fitParent = true, maxLength = 8, \
	hAlign = 1.0, low = minval*10000, high = maxval*10000, val = opts[optind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	}), \
Button(forceFitH = true, text = "?", \
	disabled = !info[0], \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})

#define ROW_DDOWN(optind, optlabel, lister) \
Label(text = optlabel, hAlign = 0.0), \
DropDownList( \
	fitParent = true, \
	maxwidth = 14_em, \
	data = lister, \
	selectedValue = opts[optind], \
	onSelectFunc = [&](int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	} \
), \
DummyWidget()

#define ROW_DDOWN_I(optind, optlabel, lister, info) \
Label(text = optlabel, hAlign = 0.0), \
DropDownList( \
	fitParent = true, \
	maxwidth = 14_em, \
	data = lister, \
	selectedValue = opts[optind], \
	onSelectFunc = [&](int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
	} \
), \
Button(forceFitH = true, text = "?", \
	disabled = !info[0], \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})

#define FONT_ROW_DDOWN(optind, scaleind, optlabel, lister, maxscale) \
Label(text = optlabel, hAlign = 0.0), \
DropDownList( \
	fitParent = true, \
	maxwidth = 14_em, \
	data = lister, \
	selectedValue = opts[optind], \
	onSelectFunc = [&](int32_t val) \
	{ \
		opts[optind] = val; \
		opt_changed[optind] = true; \
		preview_font(val, opts[scaleind]); \
	} \
), \
Button(text = "Prev", \
	forceFitH = true, \
	onPressFunc = [&]() \
	{ \
		preview_font(opts[optind], opts[scaleind]); \
	}), \
TextField(type = GUI::TextField::type::INT_DECIMAL, \
	fitParent = true, \
	hAlign = 1.0, low = 1, high = maxscale, val = opts[scaleind], \
	minwidth = 4.5_em, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		preview_font(opts[optind], val); \
		opts[scaleind] = val; \
		opt_changed[scaleind] = true; \
	})

//}

//{ Listers

//}
FONT* scale_font(FONT* f, int scale); //fonts.cpp
static bool do_prevscale = true;
static int prevfont = 0, prevscale = 1;
void ZCOptionsDialog::preview_font()
{
	static FONT* tempfont = nullptr;
	int scale = do_prevscale ? prevscale : 1;
	if(tempfont)
		destroy_font(tempfont);
	tempfont = scale_font(get_zc_font(prevfont), scale);
	fprev->setFont(tempfont);
	char buf[512];
	sprintf(buf, "Font Preview: %s (%d) [x%d]", get_zc_fontname(prevfont), prevfont, scale);
	fprev_lab->setText(buf);
}
void ZCOptionsDialog::preview_font(int fontind, int scale)
{
	prevfont = fontind;
	prevscale = scale;
	preview_font();
}

size_t tabpos1 = 0, tabpos2 = 0, tabpos3 = 0;
std::shared_ptr<GUI::Widget> ZCOptionsDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	static GUI::ListData fontlist;
	fontlist = GUI::ZCListData::fonts(false,true,true).filter(
		[](GUI::ListItem& itm)
		{
			return !isBrokenFont(itm.value);
		}
	);
	std::shared_ptr<GUI::TabRef> fontstab;
	
	fontstab = TabRef(name = "Fonts",
		Row(
			Column(
				Rows<4>(
					DummyWidget(colSpan=3),
					Row(
						Label(text = "Scale:"),
						INFOBTN("Each row has a different max scale, most are 2 or 3 max. Textbox is 5 max."
							"\nWarning: Take care with making the 'Dialog' or 'GUI' fonts too large, as this may cause"
							" the options dialog to become unusable, requiring you to manually edit 'zquest.cfg' to fix.")
					),
					FONT_ROW_DDOWN(OPT_FONT_DIALOG, OPT_FONT_SCALE_DIALOG, "Dialog Font:", fontlist, 2),
					FONT_ROW_DDOWN(OPT_FONT_GUI, OPT_FONT_SCALE_GUI, "GUI Font:", fontlist, 2),
					FONT_ROW_DDOWN(OPT_FONT_TITLE, OPT_FONT_SCALE_TITLE, "Title Font:", fontlist, 3)
					//FONT_ROW_DDOWN(OPT_FONT_FAVCMD, OPT_FONT_SCALE_FAVCMD, "FavCMD Font:", fontlist, 2),
					//FONT_ROW_DDOWN(OPT_FONT_TEXTBOX, OPT_FONT_SCALE_TEXTBOX, "Textbox Font:", fontlist, 5),
					//FONT_ROW_DDOWN(OPT_FONT_TTIP, OPT_FONT_SCALE_TTIP, "Tooltip Font:", fontlist, 3),
					//FONT_ROW_DDOWN(OPT_FONT_INFO, OPT_FONT_SCALE_INFO, "Info Font:", fontlist, 3)
				),
				Row(
					Button(text = "Default",
						onClick = message::RELOAD,
						vAlign = 0.0,
						onPressFunc = [&]()
						{
							bool doclear = false;
							AlertDialog("Default Fonts",
								"Reset all font dropdowns to defaults?",
								[&](bool ret,bool)
								{
									doclear = ret;
								}).show();
							if(!doclear) return;
							
							for(int q = OPT_FIRSTFONT; q <= OPT_LASTFONT; ++q)
							{
								int realq = q-OPT_FIRSTFONT;
								opts[q] = get_def_fontid(q-OPT_FIRSTFONT);
								opt_changed[q] = true;
							}
							
							for(int q = OPT_FIRST_FONTSCALE; q <= OPT_LAST_FONTSCALE; ++q)
							{
								opts[q] = 1;
								opt_changed[q] = true;
							}
							
							preview_font(opts[OPT_FONT_DIALOG], 1);
						}),
					Checkbox(checked = do_prevscale,
						text = "Preview Scale",
						hAlign = 0.0,
						onToggleFunc = [&](bool state)
						{
							do_prevscale = !do_prevscale;
							preview_font();
						})
				),
				Row(
					topPadding = 0.5_em,
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
			),
			Column(
				fprev_lab = Label(text = "Font Preview", hAlign = 0.0, fitParent = true),
				fprev = Label(text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,"
					" sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim"
					" ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip"
					" ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate"
					" velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat"
					" cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id"
					" est laborum. Duis nec venenatis ipsum. Aenean sed malesuada velit. Vivamus"
					" eget cursus lacus. Pellentesque pharetra dui eget consequat posuere."
					" Quisque vel semper enim."
					"\n\n! @ # $ % ^ && * ( ) [ ] { } + - = | \\ < > , : ; \" ' ~",
					framed = true, padding = 5_px,
					maxLines = 30, height = 25_em,
					width = 300_px, maxwidth = 300_px, fitParent = true)
			)
		)
	);
	
	std::shared_ptr<GUI::Window> window = Window(
		title = "ZPlayer Options",
		onClose = message::CANCEL,
		Column(
			TabPanel(ptr = &tabpos1,
				minwidth = 360_px,
				
				fontstab
			),
			Row(
				topPadding = 0.5_em,
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
	
	preview_font(opts[OPT_FONT_DIALOG], opts[OPT_FONT_SCALE_DIALOG]);
	return window;
}

bool ZCOptionsDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			saveOptions();
			return true;
		case message::CANCEL:
			return true;
		case message::RELOAD:
			rerun_dlg = true;
			return true;
	}
	return false;
}

