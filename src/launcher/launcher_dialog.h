#ifndef ZC_LAUNCHER_DIALOG_H
#define ZC_LAUNCHER_DIALOG_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <gui/drop_down_list.h>
#include <gui/button.h>
#include <gui/window.h>
#include <functional>

class LauncherDialog: public GUI::Dialog<LauncherDialog>
{
public:
	enum class message
	{
		ZC, ZQ, EXIT
	};

	LauncherDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	
private:
	bool load_theme(char const* themefile);
	int32_t launcher_on_tick();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_module_zc, tf_module_zq, tf_module_zcl, tf_savefile;
	std::shared_ptr<GUI::TextField> tf_theme[3];
	std::shared_ptr<GUI::Button> btn_save[3];
	std::shared_ptr<GUI::Label> lbl_theme_error;
	std::shared_ptr<GUI::DropDownList> ddl_res;
};

#endif

