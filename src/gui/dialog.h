#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "base/zapp.h"
#include "dialog_message.h"
#include "gui/dialog_runner.h"
#include "gui/widget.h"
#include <base/gui.h>
#include <dialog/common.h>
#include <memory>

namespace GUI
{

template<typename T>
class Dialog
{
public:
	virtual ~Dialog() {}

	/* Creates and initializes the dialog's window. */
	virtual std::shared_ptr<Widget> view()=0;
	
	DialogRunner runner;
	bool rerun_dlg;
	
	inline void show()
	{
		if (is_headless() || is_ci())
			return;

		clear_keybuf();
		auto oz = gui_mouse_z();
		runner = DialogRunner();
		rerun_dlg = false;
		popup_zqdialog_start();
		runner.run(*static_cast<T*>(this));
		while(rerun_dlg)
		{
			rerun_dlg = false;
			runner.clear();
			runner.run(*static_cast<T*>(this));
		}
		popup_zqdialog_end();
		position_mouse_z(oz);
	}
	
	inline void pendDraw()
	{
		runner.pendDraw();
	}
	
	inline void forceDraw()
	{
		runner.forceDraw();
	}
	
	inline void close()
	{
		runner.done = true;
	}

	inline void refresh_dlg()
	{
		rerun_dlg = true;
		close();
	}

	/* Subclasses must define an int32_t-convertible type called `message`
	 * and implement:
	 * bool handleMessage(const DialogMessage<message>&)
	 */
};

}

#endif
