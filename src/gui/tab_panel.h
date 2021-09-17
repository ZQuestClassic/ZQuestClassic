#ifndef ZC_GUI_TABPANEL_H
#define ZC_GUI_TABPANEL_H

#include "widget.h"
#include "dialog_ref.h"
#include "tab.h"
#include "../tab_ctl.h"
#include <memory>
#include <vector>

namespace GUI
{

class TabPanel: public Widget
{
public:
	TabPanel();

	/* Adds a tab to the panel. */
	inline void addTab(std::shared_ptr<Tab> newTab)
	{
		tabs.emplace_back(std::move(newTab));
	}

private:
	std::vector<TABPANEL> jwinTabs;
	std::vector<std::shared_ptr<Tab>> tabs;
	size_t visibleTab;
	DialogRef alDialog;

	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;

	friend class Tab;
};

}

#endif
