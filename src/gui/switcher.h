#ifndef ZC_GUI_SWITCHER_H_
#define ZC_GUI_SWITCHER_H_

#include "gui/widget.h"
#include <vector>

namespace GUI
{

/* A container that displays any one of multiple widgets. Used to dynamically
 * change the layout, e.g. to switch between a TextField and a DropDownList
 * when some selection is changed.
 */
class Switcher: public Widget
{
public:
	Switcher();

	void add(std::shared_ptr<Widget> child) noexcept
	{
		children.emplace_back(std::move(child));
	}

	/* Makes the widget with the given index visible, hiding all others. */
	void switchTo(size_t index);

	/* Returns the index of the currently visible widget. */
	inline size_t getCurrentIndex() const
	{
		return visibleChild;
	}

	/* Returns the widget at the given index, cast to the specified type. */
	template<typename T>
	std::shared_ptr<T> get(size_t index)
	{
		auto ret = std::dynamic_pointer_cast<T>(children.at(index));
		assert(ret);
		return ret;
	}

private:
	std::vector<std::shared_ptr<Widget>> children;
	size_t visibleChild;

	/* Sets or unsets D_HIDDEN for each of the child's DIALOGs. */
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
