#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "dialog_message.h"
#include "dialog_runner.h"
#include "widget.h"
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

	/* Subclasses must define an int32_t-convertible type called `message`
	 * and implement:
	 * bool handleMessage(const DialogMessage<message>&)
	 */
};

}

#endif
