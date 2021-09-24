#ifndef ZC_GUI_TAB_H
#define ZC_GUI_TAB_H

#include "widget.h"
#include <memory>
#include <string>
#include <utility>

namespace GUI
{

class Tab: public Widget
{
public:
	inline void setTitle(std::string newTitle)
	{
		title = std::move(newTitle);
	}

	inline void setContent(std::shared_ptr<Widget> newContent) noexcept
	{
		content = std::move(newContent);
	}

private:
	std::string title;
	std::shared_ptr<Widget> content;

	/* Returns the title as a char* for jwin_tab_proc. */
	char* getTitle();

	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;

	friend class TabPanel;
};

}

#endif
