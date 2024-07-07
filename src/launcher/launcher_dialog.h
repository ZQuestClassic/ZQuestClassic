#ifndef ZC_LAUNCHER_DIALOG_H_
#define ZC_LAUNCHER_DIALOG_H_

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
		ZC, ZQ, ZU, ZU_RELEASE_NOTES, ZU_RELEASE_NOTES_NEXT, EXIT, SUPPORT
	};

	LauncherDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	
private:
	bool load_theme(char const* themefile);
	int32_t launcher_on_tick();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_savefile;
	std::shared_ptr<GUI::TextField> tf_theme[3];
	std::shared_ptr<GUI::Button> btn_save[3];
	std::shared_ptr<GUI::Button> btn_download_update;
	std::shared_ptr<GUI::Button> btn_release_notes;
	std::shared_ptr<GUI::Label> lbl_theme_error;
	std::shared_ptr<GUI::DropDownList> ddl_res;
};

#endif

