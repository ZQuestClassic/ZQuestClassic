#include "compilesetting.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "info.h"
#include "zq/zquest.h"
#include "zc/ffscript.h"
#include "core/qrs.h"

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

static const GUI::ListData list_off_warn_err
{
	{ "Nothing", 0 },
	{ "Warn", 3 },
	{ "Error", 2 },
};

static const GUI::ListData list_deprecated_features
{
	{ "Off", 0 },
	{ "On", 1 },
	{ "Warn", 3 },
};

static const GUI::ListData list_auto_off_on
{
	{ "Auto", -1 },
	{ "Off", 0 },
	{ "On", 1 },
};

GUI::ListData compileSettingList
{
	{ "Disable Tracing", qr_PARSER_NO_LOGGING, "OPTION - Disables 'Trace()', 'printf()', and similar commands." },
	{ "Switch/case of strings is case-insensitive", qr_PARSER_STRINGSWITCH_INSENSITIVE, "OPTION - disables 'switch(string)' case sensitivity" },
};

void CompileSettingsDlg::load()
{
	dd_cfg[0] = zc_get_config("Compiler","NO_ERROR_HALT",0,App::zscript);
	dd_cfg[1] = zc_get_config("Compiler","WARN_DEPRECATED",0,App::zscript);
	dd_cfg[2] = zc_get_config("Compiler","ON_MISSING_RETURN",2,App::zscript);
	dd_cfg[3] = zc_get_config("Compiler","LEGACY_ARRAYS",3,App::zscript);
	old_timeout_secs = timeout_secs = zc_get_config("Compiler","compiler_timeout",30,App::zscript);
	memcpy(old_dd_cfg,dd_cfg,sizeof(dd_cfg));
	
	strcpy(include_str,FFCore.includePathString);
	include_str[MAX_INCLUDE_PATH_CHARS-1] = 0;
	
	for(size_t q = 0; q < compileSettingList.size(); ++q)
	{
		if(q > 8*sizeof(qst_cfg)) //sanity... probably not really necessary, but just to be safe -Em
		{
			InfoDialog("DEV ERROR","Not enough space for all quest-specific QRs!").show();
			break;
		}
		set_bit(qst_cfg,q,get_qr(compileSettingList.getValue(q)));
	}
	local_compiler_settings = qst_compiler_settings;
}
void CompileSettingsDlg::save()
{
	if(dd_cfg[0] != old_dd_cfg[0])
		zc_set_config("Compiler","NO_ERROR_HALT",dd_cfg[0],App::zscript);
	if(dd_cfg[1] != old_dd_cfg[1])
		zc_set_config("Compiler","WARN_DEPRECATED",dd_cfg[1],App::zscript);
	if(dd_cfg[2] != old_dd_cfg[2])
		zc_set_config("Compiler","ON_MISSING_RETURN",dd_cfg[2],App::zscript);
	if(dd_cfg[3] != old_dd_cfg[3])
		zc_set_config("Compiler","LEGACY_ARRAYS",dd_cfg[3],App::zscript);
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
	
	vector<QSTCompilerSetting> ids_to_remove;
	for (auto [id, val] : local_compiler_settings)
	{
		switch (id)
		{
			case QSTCompilerSetting::DEFAULT_STATIC_SCRIPT_MEMBERS:
				if (val == -1)
					ids_to_remove.push_back(id);
				break;
		}
	}
	for (auto id : ids_to_remove)
		local_compiler_settings.erase(id);
	
	qst_compiler_settings = local_compiler_settings;
	
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
	
	shared_ptr<GUI::Grid> qrs_grid;
	
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
						_d,
						//
						Label(text = "On Deprecated:", hAlign = 1.0),
						DropDownList(data = list_off_warn_err,
							fitParent = true,
							selectedValue = dd_cfg[1],
							onSelectFunc = [&](int32_t val)
							{
								dd_cfg[1] = val;
							}
						),
						INFOBTN("How to behave when a deprecated function is used. These are 'old' functions that are"
							" no longer intended to be used, as there are better alternatives. If you are developing a"
							" new quest with only new scripts, there is no real reason not to enable this. Additionally,"
							" this setting can aid in upgrading older scripts to use newer ZScript functions.\n\n"
							"Most deprecated functions will NOT be found in the latest documentation at all."),
						//
						Label(text = "On Missing Return:", hAlign = 1.0),
						DropDownList(data = list_off_warn_err,
							fitParent = true,
							selectedValue = dd_cfg[2],
							onSelectFunc = [&](int32_t val)
							{
								dd_cfg[2] = val;
							}
						),
						INFOBTN("How to behave when a function is missing a return statement. If a non-void function"
							" ends without returning a value, the return value will be random garbage. To avoid this,"
							" the compiler can warn or error when it detects missing return statements."
							"\n\nThe compiler should be quite smart about detecting these situations."),
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
						INFOBTN("The time ZQ waits before the parser 'times out'. 0 for no timeout."),
						//
						Label(text = "Legacy Arrays", hAlign = 1.0),
						DropDownList(data = list_deprecated_features,
							fitParent = true,
							selectedValue = dd_cfg[3],
							onSelectFunc = [&](int32_t val)
							{
								dd_cfg[3] = val;
							}
						),
						INFOBTN("Allows array variables to be declared and used without an explicit array type."
								"\nFor example, `int arr = {1, 2};` is a legacy array, but `int[] arr = {1, 2};`"
								" is not."
								" \nOnly applies to types that cannot be `Own`'d - custom objects, bitmaps, etc. cannot"
								" be stored using legacy array syntax."
							"\nNOTE: It is recommended to keep this `OFF`, except possibly for older code.")
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
					qrs_grid = Rows<2>(),
					Rows<3>(
						Label(text = "Default Static Script Members", hAlign = 1.0),
						DropDownList(data = list_auto_off_on,
							fitParent = true,
							selectedValue = local_compiler_settings.contains(QSTCompilerSetting::DEFAULT_STATIC_SCRIPT_MEMBERS)
								? local_compiler_settings[QSTCompilerSetting::DEFAULT_STATIC_SCRIPT_MEMBERS] : -1,
							onSelectFunc = [&](int32_t val)
							{
								local_compiler_settings[QSTCompilerSetting::DEFAULT_STATIC_SCRIPT_MEMBERS] = val;
							}
						),
						INFOBTN(
							"AUTO: Automatically picks ON or OFF. Currently, always picks ON; this will eventually change."
							"\nOFF: Scripts will default to members being non-static, requiring the 'static' keyword to make them static."
							"\nON: Scripts will default to members being static, requiring the 'nonstatic' keyword to make them non-static."
						)
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
	int qr_index = 0;
	compileSettingList.for_all_items([&](auto const& itm)
	{
		int qr = qr_index++;
		
		if (itm.info.empty())
			qrs_grid->add(DINFOBTN());
		else
			qrs_grid->add(INFOBTN(itm.info));
		
		qrs_grid->add(
			Checkbox(
				text = itm.text, hAlign = 0.0,
				checked = get_qr(qr),
				onToggleFunc = [&, qr](bool state)
				{
					set_bit(qst_cfg, qr, state);
				})
		);
	});
	return window;
}

bool CompileSettingsDlg::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::REFR_INFO:
			rerun_dlg = true;
			return true;
		case message::OK:
			save();
			mark_save_dirty();
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

