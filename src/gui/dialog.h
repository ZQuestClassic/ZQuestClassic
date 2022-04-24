#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "dialog_message.h"
#include "dialog_runner.h"
#include "widget.h"
#include <dialog/common.h>
#include <memory>

#ifdef IS_PLAYER
extern bool is_sys_pal;
void system_pal();
void game_pal();
void update_hw_screen(bool force);
#endif

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
		#ifdef IS_PLAYER
		bool p = !is_sys_pal;
		if(p) system_pal();
		update_hw_screen();
		#endif
		
		runner = DialogRunner();
		runner.run(*static_cast<T*>(this));
		
		#ifdef IS_PLAYER
		if(p) game_pal();
		#endif
		position_mouse_z(0);
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
