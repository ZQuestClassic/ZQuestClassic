#include "theme_editor.h"
#include "dialog/common.h"
#include "dialog/alert.h"
#include "base/jwinfsel.h"
#include <gui/builder.h>
#include "launcher/launcher.h"

extern int32_t zq_screen_w, zq_screen_h;
extern PALETTE RAMpal;

extern char temppath[4096];
char zthemepath[4096] = {0};

ThemeEditor::ThemeEditor(char* buf) : saved_path(buf)
{
	if(buf) buf[0] = 0;
}


static int32_t queue_revert = 0;
int32_t ThemeEditor::theme_edit_on_tick()
{
	if(queue_revert > 0)
	{
		if(!--queue_revert)
		{
			AlertDialog("Theme Check",
				"Would you like to revert?",
				[&](bool ret,bool)
				{
					if(ret)
					{
						set_palette(temp_pal);
						jwin_set_colors(t_jwin_pal);
					}
					else
					{
						get_palette(temp_pal);
						memcpy(t_jwin_pal, jwin_pal, sizeof(jwin_pal));
					}
				}, "Revert", "Keep", 60*4, true).show();
			pendDraw();
		}
	}
	return ONTICK_CONTINUE;
}


#define RGB_ROW(ind) \
Label(text = "RGB " + std::to_string(ind) + ":"), \
tf_red.add(TextField(fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = 0, high = 63, val = work_pal[ind].r, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		work_pal[ind].r = val; \
	})), \
tf_green.add(TextField(fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = 0, high = 63, val = work_pal[ind].g, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		work_pal[ind].g = val; \
	})), \
tf_blue.add(TextField(fitParent = true, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = 0, high = 63, val = work_pal[ind].b, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		work_pal[ind].b = val; \
	})), \
ColorSel(read_only = true, val = ind, width = 4.5_em)

#define JC_ROW(jc_const, info) \
Label(text = #jc_const), \
tf_jc.add(TextField(fitParent = true, minwidth = 2_em, \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = 1, high = 8, val = jwin_pal[jc_const], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		jwin_pal[jc_const] = val; \
	})), \
Button(forceFitH = true, text = "?", \
	disabled = !info[0], \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})


std::shared_ptr<GUI::Widget> ThemeEditor::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	queue_revert = 0;
	
	memcpy(t_jwin_pal, jwin_pal, sizeof(jwin_pal));
	memcpy(restore_jwin_pal, jwin_pal, sizeof(jwin_pal));
	get_palette(restore_pal);
	get_palette(temp_pal);
	get_palette(work_pal);
	for(auto q = 1; q <= 8; ++q)
	{
		temp_pal[q] = temp_pal[dvc(q)];
		work_pal[q] = work_pal[dvc(q)];
	}
	for(auto q = 0; q < jcMAX; ++q)
	{
		t_jwin_pal[q] -= dvc(0);
		jwin_pal[q] -= dvc(0);
	}
	set_palette(temp_pal);
	jwin_set_colors(jwin_pal);
	
	window = Window(
		title = "Theme Editor",
		width = 0_px + zq_screen_w,
		height = 0_px + zq_screen_h,
		onClose = message::CANCEL,
		use_vsync = true,
		onTick = [&](){return theme_edit_on_tick();},
		shortcuts={
			Esc=message::CANCEL
		},
		Column(
			Row(padding = 0_px,
				Rows<5>(
					RGB_ROW(1),
					RGB_ROW(2),
					RGB_ROW(3),
					RGB_ROW(4),
					RGB_ROW(5),
					RGB_ROW(6),
					RGB_ROW(7),
					RGB_ROW(8)
				),
				Rows<3>(
					JC_ROW(jcBOX, "The general gui background color"),
					JC_ROW(jcLIGHT, ""),
					JC_ROW(jcMEDLT, ""),
					JC_ROW(jcMEDDARK, ""),
					JC_ROW(jcDARK, ""),
					JC_ROW(jcBOXFG, ""),
					JC_ROW(jcTITLEL, "The left color used in titlebars"),
					JC_ROW(jcTITLER, "The right color used in titlebars"),
					JC_ROW(jcTITLEFG, "The titlebar text color"),
					JC_ROW(jcTEXTBG, "The text bg color"),
					JC_ROW(jcTEXTFG, "The main text color")
				),
				Rows<3>(
					JC_ROW(jcSELBG, "The 'selected' background color"),
					JC_ROW(jcSELFG, "The 'selected' text color"),
					JC_ROW(jcCURSORMISC, ""),
					JC_ROW(jcCURSOROUTLINE, "The outline of the cursor"),
					JC_ROW(jcCURSORLIGHT, ""),
					JC_ROW(jcCURSORDARK, ""),
					JC_ROW(jcALT_TEXTFG, "Used as an alternate text color, ex. for read-only textfields"),
					JC_ROW(jcALT_TEXTBG, "Used as an alternate text bg color, ex. for read-only textfields"),
					JC_ROW(jcDISABLED_FG, "Used as a foreground color for disabled objects."),
					JC_ROW(jcDISABLED_BG, "Used as a background color for disabled objects.")
				)
			),
			Row(padding = 0_px,
				Button(text = "Preview", onPressFunc = [&]()
					{
						set_palette(work_pal);
						jwin_set_colors(jwin_pal);
						queue_revert = 2;
					}),
				Button(text = "Revert", onPressFunc = [&]()
					{
						memcpy(work_pal, temp_pal, sizeof(PALETTE));
						memcpy(jwin_pal, t_jwin_pal, sizeof(jwin_pal));
						tf_red.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(work_pal[8-ind].r);
							});
						tf_green.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(work_pal[8-ind].g);
							});
						tf_blue.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(work_pal[8-ind].b);
							});
						tf_jc.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(jwin_pal[ind]);
							});
						set_palette(work_pal);
						jwin_set_colors(jwin_pal);
					}),
				Button(text = "Hard Revert", onPressFunc = [&]()
					{
						memcpy(work_pal, restore_pal, sizeof(PALETTE));
						memcpy(temp_pal, restore_pal, sizeof(PALETTE));
						memcpy(jwin_pal, restore_jwin_pal, sizeof(jwin_pal));
						memcpy(t_jwin_pal, restore_jwin_pal, sizeof(jwin_pal));
						for(auto q = 1; q <= 8; ++q)
						{
							work_pal[q] = work_pal[dvc(q)];
							temp_pal[q] = temp_pal[dvc(q)];
						}
						for(auto q = 0; q < jcMAX; ++q)
						{
							jwin_pal[q] -= dvc(0);
							t_jwin_pal[q] -= dvc(0);
						}
						tf_red.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(work_pal[8-ind].r);
							});
						tf_green.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(work_pal[8-ind].g);
							});
						tf_blue.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(work_pal[8-ind].b);
							});
						tf_jc.forEach([&](std::shared_ptr<GUI::TextField> tfield, size_t ind)
							{
								tfield->setVal(jwin_pal[ind]);
							});
						set_palette(work_pal);
						jwin_set_colors(jwin_pal);
					})
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(focused = true,
					text = "Save",
					minwidth = 90_px,
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

bool ThemeEditor::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			if(saved_path) saved_path[0] = 0;
			//Save?
			char path[4096] = {0};
			if(getname("Save Theme", "ztheme", NULL, zthemepath, false))
			{
				relativize_path(path, temppath);
				if(saved_path)
					strcpy(saved_path,path);
				
				for(auto q = strlen(temppath)-1; q > 0 && !(temppath[q] == '/' || temppath[q] == '\\'); --q)
				{
					temppath[q] = 0;
				}
				strcpy(zthemepath, temppath);
			}
			else return false;
			//Save things
			for(auto q = 1; q <= 8; ++q)
			{
				work_pal[dvc(q)] = work_pal[q];
			}
			for(auto q = 0; q < jcMAX; ++q)
			{
				jwin_pal[q] += dvc(0);
			}
			save_themefile(path, work_pal);
			//Restore
			memcpy(jwin_pal, restore_jwin_pal, sizeof(jwin_pal));
			jwin_set_colors(jwin_pal);
			forceDraw();
			update_hw_screen();
			set_palette(restore_pal);
			return true;
		}
		case message::CANCEL:
			if(saved_path) saved_path[0] = 0;
			set_palette(restore_pal);
			memcpy(jwin_pal, restore_jwin_pal, sizeof(jwin_pal));
			jwin_set_colors(jwin_pal);
			return true;
	}
	return false;
}
