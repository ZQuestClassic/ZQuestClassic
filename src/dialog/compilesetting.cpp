#include "compilesetting.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "info.h"
#include "zq/zquest.h"
#include "zc/ffscript.h"
#include "base/qrs.h"

extern FFScript FFCore;
extern std::vector<std::string> ZQincludePaths;

void write_includepaths();

void call_compile_settings()
{
	CompileSettingsDlg().show();
}

static const GUI::ListData list_halt
{
	{ "Halt", 0 },
	{ "Do Not Halt", 1 }
};

static const GUI::ListData list_headguard
{
	{ "Disable", 0 },
	{ "Enable", 1 },
	{ "Error, Enable", 2 },
	{ "Warn, Enable", 3 }
};

static const GUI::ListData list_ondepr
{
	{ "Ignore", 0 },
	{ "Warn", 1 },
	{ "Error", 2 }
};

GUI::ListData compileSettingList
{
	{ "2.50 Division Truncation", qr_PARSER_250DIVISION, "COMPAT - This replicates an old bug with division." },
	{ "Disable Tracing", qr_PARSER_NO_LOGGING, "OPTION - Disables 'Trace()', 'printf()', and similar commands." },
	{ "Short-Circuit Boolean Operations", qr_PARSER_SHORT_CIRCUIT, "COMPAT REVERSE - applies short-circuit." },
	{ "2.50 Value of 'true' is 0.0001", qr_PARSER_BOOL_TRUE_DECIMAL, "COMPAT - replicates old behavior possibly needed for ZASM" },
	{ "True MAX_INT sizing", qr_PARSER_TRUE_INT_SIZE, "COMPAT REVERSE - Uses 214748.3648 as the max value, instead of 214747 (similar for negative min value)" },
	//{ "", qr_PARSER_FORCE_INLINE, "" }, //Unimplemented
	{ "Binary Operations use true 32-bit Int", qr_PARSER_BINARY_32BIT, "COMPAT - treats all binary operations as 'long' type" },
	{ "Switch/case of strings is case-insensitive", qr_PARSER_STRINGSWITCH_INSENSITIVE, "OPTION - disables 'switch(string)' case sensitivity" },
};

void CompileSettingsDlg::load()
{
	dd_cfg[0] = zc_get_config("Compiler","NO_ERROR_HALT",0,App::zscript);
	dd_cfg[1] = zc_get_config("Compiler","HEADER_GUARD",1,App::zscript);
	dd_cfg[2] = zc_get_config("Compiler","WARN_DEPRECATED",0,App::zscript);
	old_timeout_secs = timeout_secs = zc_get_config("Compiler","compiler_timeout",30,App::zscript);
	memcpy(old_dd_cfg,dd_cfg,sizeof(dd_cfg));
	
	strcpy(include_str,FFCore.includePathString);
	include_str[MAX_INCLUDE_PATH_CHARS-1] = 0;
	
	for(size_t q = 0; q < compileSettingList.size(); ++q)
	{
		if(q > 8*sizeof(qst_cfg)) //sanity... probably not really necessary, but just to be safe -Em
		{
			displayinfo("DEV ERROR","Not enough space for all quest-specific QRs!");
			break;
		}
		set_bit(qst_cfg,q,get_qr(compileSettingList.getValue(q)));
	}
}
void CompileSettingsDlg::save()
{
	if(dd_cfg[0] != old_dd_cfg[0])
		zc_set_config("Compiler","NO_ERROR_HALT",dd_cfg[0],App::zscript);
	if(dd_cfg[1] != old_dd_cfg[1])
		zc_set_config("Compiler","HEADER_GUARD",dd_cfg[1],App::zscript);
	if(dd_cfg[2] != old_dd_cfg[2])
		zc_set_config("Compiler","WARN_DEPRECATED",dd_cfg[2],App::zscript);
	if(timeout_secs != old_timeout_secs)
		zc_set_config("Compiler","compiler_timeout",timeout_secs,App::zscript);
	
	include_str[MAX_INCLUDE_PATH_CHARS-1] = 0;
	memset(FFCore.includePathString,0,sizeof(FFCore.includePathString));
	strcpy(FFCore.includePathString,include_str);
	
	for(size_t q = 0; q < compileSettingList.size(); ++q)
	{
		if(q > 8*sizeof(qst_cfg)) //match the above sanity check -Em
			break;
		set_qr(compileSettingList.getValue(q),get_bit(qst_cfg,q));
	}
	
	FFCore.updateIncludePaths();
	ZQincludePaths = FFCore.includePaths;
	write_includepaths();
}

CompileSettingsDlg::CompileSettingsDlg()
{
	load();
}

static size_t compiletab = 0;
std::shared_ptr<GUI::Widget> CompileSettingsDlg::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	window = Window(
		title = "ZScript Compiler Settings",
		onClose = message::CANCEL,
		info = "Settings used by the compiler when compiling scripts."
			"\nFor Quest Settings: 'COMPAT' options should generally be off unless you have a reason,"
			" and 'COMPAT REVERSE' options should generally be ON unless you have a reason."
			" 'OPTION' options are entirely user discretion.",
		Column(
			TabPanel(
				ptr = &compiletab,
				TabRef(name = "Global Settings", Column(
					Rows<3>(hAlign = 0.0,
						Label(text = "On Error:", hAlign = 1.0),
						DropDownList(data = list_halt,
							fitParent = true,
							selectedValue = dd_cfg[0],
							onSelectFunc = [&](int32_t val)
							{
								dd_cfg[0] = val;
							}
						),
						INFOBTN("Whether to halt on the first error, or continue to try to collect more error messages."),
						//
						Label(text = "Header Guard:", hAlign = 1.0),
						DropDownList(data = list_headguard,
							fitParent = true,
							selectedValue = dd_cfg[1],
							onSelectFunc = [&](int32_t val)
							{
								dd_cfg[1] = val;
							}
						),
						INFOBTN("How to behave when the same file is included multiple times.\n"
							"Disable means it will just include multiple times, often causing duplication errors."
							"Enable will ignore duplicates."
							"Error/Warn will ignore duplicates, but throw an error/warning respectively."),
						//
						Label(text = "On Deprecated:", hAlign = 1.0),
						DropDownList(data = list_ondepr,
							fitParent = true,
							selectedValue = dd_cfg[2],
							onSelectFunc = [&](int32_t val)
							{
								dd_cfg[2] = val;
							}
						),
						INFOBTN("How to behave when a deprecated function is used. These are 'old' functions that are"
							" no longer intended to be used, as there are better alternatives. If you are developing a"
							" new quest with only new scripts, there is no real reason not to enable this. Additionally,"
							" this setting can aid in upgrading older scripts to use newer ZScript functions.\n\n"
							"Most deprecated functions will NOT be found in the latest documentation at all."),
						//
						Label(text = "Timeout Seconds", hAlign = 1.0),
						TextField(type = GUI::TextField::type::INT_DECIMAL,
							forceFitW = true,
							hAlign = 1.0, low = 0, high = 3600, val = timeout_secs,
							minwidth = 4.5_em,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								timeout_secs = val;
							}),
						INFOBTN("The time ZQ waits before the parser 'times out'. 0 for no timeout.")
					),
					Label(text = "Include Paths", hAlign = 0.0),
					TextField(
						fitParent = true,
						width = 375_px,
						maxLength = MAX_INCLUDE_PATH_CHARS,
						text = include_str,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
						{
							std::string name(str);
							strncpy(include_str, name.c_str(), MAX_INCLUDE_PATH_CHARS-1);
							include_str[MAX_INCLUDE_PATH_CHARS-1] = 0;
							include_label->setText(include_str);
						}
					),
					// The old dialog used a 'jwin_textbox_proc', which automatically allowed scrolling
					// in the case that it was too big to fit... Not bothering with that now. -Em
					include_label = Label(
						forceFitW = true,
						framed = true,
						minheight = 5_em,
						vPadding = 6_px,
						hMargins = DEFAULT_PADDING,
						hPadding = DEFAULT_PADDING*2,
						text = include_str
					)
				)),
				TabRef(name = "Quest Settings", Column(
					QRPanel(
						padding = 3_px,
						onToggle = message::TOGGLE_QUEST_CFG,
						indexed = true,
						initializer = qst_cfg,
						count = 14,
						data = compileSettingList
					)
				))
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool CompileSettingsDlg::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::TOGGLE_QUEST_CFG:
			toggle_bit(qst_cfg,msg.argument);
			break;
		case message::OK:
			save();
			saved = false;
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

