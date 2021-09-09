#ifndef ZC_GUI_TOPLEVEL_H
#define ZC_GUI_TOPLEVEL_H

#include "widget.h"
#include "key.h"
#include <initializer_list>
#include <vector>

struct DIALOG;

namespace GUI
{

class DialogRunner;

class TopLevelWidget: public Widget
{
public:
	virtual ~TopLevelWidget() {}

	/* Add a keyboard shortcut. */
	template<typename T>
	inline void onKey(ShortcutKey k, T message)
	{
		shortcuts.emplace_back(KeyboardShortcut {
			k.get(), static_cast<int>(message)
		});
	}

	/* Add a bunch of shortcuts at once. These will be added
	 * to any shortcuts already defined.
	 */
	void addShortcuts(std::initializer_list<KeyboardShortcut>&& scList);

protected:
	/* Insert keyboard shortcut DIALOGs into the array. This should be called
	 * by subclasses after inserting their own and their children's DIALOGs.
	 */
	void realizeKeys(DialogRunner& runner);
	int onEvent(int event, MessageDispatcher sendMessage) override;

private:
	std::vector<KeyboardShortcut> shortcuts;

	static int proc(int msg, DIALOG* d, int c);
};

}

#endif
