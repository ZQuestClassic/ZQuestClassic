#ifndef ZC_GUI_DIALOGRUNNER_H
#define ZC_GUI_DIALOGRUNNER_H

#include "dialog.h"
#include "dialog_message.h"
#include "dialog_ref.h"
#include "helper.h"
#include "widget.h"
#include <memory>
#include <type_traits>

namespace GUI
{

class DialogRunner
{
public:
	DialogRunner();

	template<typename T>
	void runWithArg(T& dlg)
	{
		sendMessage = [&dlg, this](int msg, MessageArg arg)
		{
			this->done = this->done ||
				dlg.handleMessage(static_cast<typename T::message>(msg), arg);
		};

		runInner(dlg.view());
	}

	template<typename T>
	void runWithoutArg(T& dlg)
	{
		sendMessage = [&dlg, this](int msg, MessageArg)
		{
			this->done = this->done ||
				dlg.handleMessage(static_cast<typename T::message>(msg));
		};

		runInner(dlg.view());
	}

	/* Add a DIALOG and connect it to its owner.
	 * This should always be called as
	 * runner.push(shared_from_this(), DIALOG { ... });
	 * Returns a DialogRef that can be used as a reference to the
	 * newly added DIALOG.
	 */
	DialogRef push(std::shared_ptr<Widget> owner, DIALOG dlg);

	/* Returns the current size of the DIALOG array. */
	inline size_t size() const
	{
		return alDialog.size();
	}

private:
	MessageDispatcher sendMessage;
	std::vector<DIALOG> alDialog;
	std::vector<std::shared_ptr<Widget>> widgets;
	int focused;
	bool redrawPending, done;

	/* Sets up the DIALOG array for a dialog so that it can be run. */
	void realize(std::shared_ptr<Widget> root);

	void runInner(std::shared_ptr<Widget> root);

	friend class DialogRef;
	friend int dialog_proc(int msg, DIALOG *d, int c);
};

// Pick either the argument or non-argument version of handleMessage()
// based on which can be called.

template<typename T>
std::enable_if_t<
	std::is_invocable_v<
		decltype(&T::handleMessage), T&, typename T::message, GUI::MessageArg
	>, void
> showDialog(T& dlg)
{
	auto dr=DialogRunner();
	dr.runWithArg(dlg);
}

template<typename T>
std::enable_if_t<
	std::is_invocable_v<
		decltype(&T::handleMessage), T&, typename T::message
	>, void
> showDialog(T& dlg)
{
	auto dr=DialogRunner();
	dr.runWithoutArg(dlg);
}

// This one just exists to produce a more helpful error message if neither
// version is correctly defined. The enable_if is just to prevent additional
// errors from ambiguity with the two above.
template<typename T, bool b=false>
std::enable_if_t<
	!std::is_invocable_v<
		decltype(&T::handleMessage), T&, typename T::message, GUI::MessageArg>
	&& !std::is_invocable_v<
		decltype(&T::handleMessage), T&, typename T::message
	>, void
> showDialog(T& dlg)
{
	ZCGUI_STATIC_ASSERT(b,
		"No valid handleMessage() implementation found.\n"
		"You must implement one of the following:\n"
		"handleMessage([DialogClass]::Message, GUI::EventArg)\n"
		"handleMessage([DialogClass]::Message)");
}


}

#endif
