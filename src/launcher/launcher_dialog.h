#ifndef ZC_LAUNCHER_DIALOG_H
#define ZC_LAUNCHER_DIALOG_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <gui/window.h>
#include <functional>

class LauncherDialog: public GUI::Dialog<LauncherDialog>
{
public:
	enum class message
	{
		ZC, ZQ, EXIT
	};

	LauncherDialog::LauncherDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	
private:
	int32_t launcher_on_tick();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_theme;
	std::shared_ptr<GUI::Label> lbl_theme_error;
};

#endif

