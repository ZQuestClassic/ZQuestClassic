#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "dialog_message.h"
#include "dialog_runner.h"
#include "widget.h"
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
		runner = DialogRunner();
		runner.run(*static_cast<T*>(this));
	}
	
	inline void pendDraw()
	{
		runner.pendDraw();
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
