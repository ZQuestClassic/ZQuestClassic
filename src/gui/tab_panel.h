#ifndef ZC_GUI_TABPANEL_H
#define ZC_GUI_TABPANEL_H

#include "widget.h"
#include "dialog_ref.h"
#include "tab.h"
#include <memory>
#include <vector>

namespace GUI
{

class TabPanel: public Widget
{
public:
	TabPanel();

	/* Adds a tab to the panel. */
	inline void addTab(std::shared_ptr<Tab> newTab) noexcept
	{
		tabs.emplace_back(std::move(newTab));
	}

private:
	std::vector<std::shared_ptr<Tab>> tabs;
	size_t selectedTab;
	DialogRef alDialog;

	void selectTab(size_t newTab);
	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;

	static int proc(int msg, DIALOG* d, int c);

	// Verious helper functions for proc.
	int clickedTab(int firstTab, int x);
	int tabsWidth();
	int visibleTabsWidth(int firstTab, int maxWidth);
	bool usesTabArrows(int maxWidth);
	int lastVisibleTab(int firstTab, int maxWidth);

	friend class Tab;
};

}

#endif
