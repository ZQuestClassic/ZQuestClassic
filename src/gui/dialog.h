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
	
	inline void show()
	{
		auto oz = gui_mouse_z();
		runner = DialogRunner();
		
		freeze_render(); runner.render_froze = true;
		runner.run(*static_cast<T*>(this));
		while(runner.rerun_dlg)
		{
			bool froze = runner.render_froze;
			runner.clear();
			if(!froze)
				freeze_render();
			runner.render_froze = true;
			runner.run(*static_cast<T*>(this));
		}
		if(runner.render_froze)
			unfreeze_render();
		
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
