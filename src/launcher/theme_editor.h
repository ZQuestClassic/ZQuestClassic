#ifndef ZC_THEME_EDIT_DIALOG_H_
#define ZC_THEME_EDIT_DIALOG_H_

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <gui/drop_down_list.h>
#include <gui/button.h>
#include <gui/window.h>
#include <functional>

class ThemeEditor: public GUI::Dialog<ThemeEditor>
{
public:
	enum class message
	{
		OK, CANCEL
	};

	ThemeEditor(char* buf = NULL);
	int32_t theme_edit_on_tick();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	
private:
	std::shared_ptr<GUI::Window> window;
	char* saved_path;
	PALETTE restore_pal, temp_pal, work_pal;
	int32_t restore_jwin_pal[jcMAX], t_jwin_pal[jcMAX];
	ALLEGRO_COLOR restore_jwin_a5_colors[9], t_jwin_a5_colors[9];
	unsigned char work_colors[9][3];
	WidgetSet<GUI::TextField> tf_red, tf_green, tf_blue, tf_jc;
};

#endif

