#ifndef ZC_GUI_TOPLEVEL_H_
#define ZC_GUI_TOPLEVEL_H_

#include "gui/widget.h"
#include "gui/key.h"
#include <initializer_list>
#include <vector>
#include <map>

struct DIALOG;

namespace GUI
{

class DialogRunner;

class TopLevelWidget: public Widget
{
public:
	virtual ~TopLevelWidget() {}
	bool isTopLevel() const override {return true;}
	
	/* Add a keyboard shortcut. */
	template<typename T>
	inline RequireMessage<T> onKey(ShortcutKey k, T message)
	{
		shortcuts[k.get()] = KeyboardShortcut {
			k.get(), static_cast<int32_t>(message)
		};
	}
	
	/* Convenience method to add a shortcut for Enter, since that's
	 * particularly common.
	 */
	template<typename T>
	inline RequireMessage<T> onEnter(T message)
	{
		shortcuts[Key::Enter.get()] = KeyboardShortcut {
			Key::Enter.get(), static_cast<int32_t>(message)
		};
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
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	
	std::string helptext; //If the first char isn't null, displays on pressing F1
private:
	std::map<uint16_t, KeyboardShortcut> shortcuts;
	
	static int32_t proc(int32_t msg, DIALOG* d, int32_t c);
	static int32_t helpproc(int32_t msg, DIALOG* d, int32_t c);
};

}

#endif
