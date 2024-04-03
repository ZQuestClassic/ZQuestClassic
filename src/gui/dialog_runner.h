#ifndef ZC_GUI_DIALOGRUNNER_H
#define ZC_GUI_DIALOGRUNNER_H

#include "dialog_message.h"
#include "gui/dialog_ref.h"
#include "gui/helper.h"
#include "gui/widget.h"
#include <memory>
#include <type_traits>

namespace GUI
{

class DialogRunner
{
public:
	template<typename T>
	void run(T& dlg)
	{
		sendMessage =
			[this, &dlg](int32_t msg, MessageArg arg, std::shared_ptr<Widget> snd)
			{
				DialogMessage<typename T::message> dm;
				dm.message = static_cast<typename T::message>(msg);
				dm.argument = arg;
				dm.sender = snd;
				this->done = this->done || dlg.handleMessage(dm);
			};
		
		std::shared_ptr<Widget> root = dlg.view();
		if(root)
		{
			realize(root);
			realized = true;
			dlg.post_realize();
			runInner(root);
		}
	}

	/* Add a DIALOG and connect it to its owner.
	 * This should always be called as
	 * runner.push(shared_from_this(), DIALOG { ... });
	 * Returns a DialogRef that can be used as a reference to the
	 * newly added DIALOG.
	 */
	DialogRef push(std::shared_ptr<Widget> owner, DIALOG dlg, bool nofocus = false);

	/* Returns the current size of the DIALOG array. */
	inline size_t size() const
	{
		return alDialog.size();
	}

	/* Returns a raw pointer to the DIALOG array for widgets that need it.
	 * This must not be called until the array is fully constructed.
	 */
	inline DIALOG* getDialogArray()
	{
		return alDialog.data();
	}

	bool isConstructed()
	{
		return realized;
	}

	bool allowDraw()
	{
		return running;
	}
	
	void pendDraw()
	{
		redrawPending = true;
	}
	
	void forceDraw()
	{
		acquire_screen();
		broadcast_dialog_message(MSG_DRAW, 0);
		release_screen();
		redrawPending = false;
	}

	void close()
	{
		done = true;
	}

private:
	std::function<void(int32_t, MessageArg, std::shared_ptr<Widget>)> sendMessage;
	std::vector<DIALOG> alDialog;
	std::vector<std::shared_ptr<Widget>> widgets;
	int32_t focused;
	bool redrawPending, done, realized, running;

	DialogRunner();
	void clear();

	/* Sets up the DIALOG array for a dialog so that it can be run. */
	void realize(std::shared_ptr<Widget> root);

	void runInner(std::shared_ptr<Widget> root);

	friend class DialogRef;
	friend int32_t dialog_proc(int32_t msg, DIALOG *d, int32_t c);
	template<typename T> friend class Dialog;
	template<typename T> friend void showDialog(T& dlg);
};

// Separate from DialogRunner due to type resolution limitations.
template<typename T>
inline void showDialog(T& dlg)
{
	auto dr=DialogRunner();
	dr.run(dlg);
}

}

#endif
