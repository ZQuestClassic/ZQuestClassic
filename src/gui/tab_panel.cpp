#include "tab_panel.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <algorithm>
#include <utility>

namespace GUI
{

TabPanel::TabPanel()
{
	fgColor = vc(0);
	bgColor = vc(15);
}

void TabPanel::applyVisibility(bool visible)
{
	if(alDialog)
	{
		if(visible)
			alDialog->flags &= ~D_HIDDEN;
		else
			alDialog->flags |= D_HIDDEN;
	}
	for(auto& tab: tabs)
		tab->applyVisibility(visible);
}

void TabPanel::calculateSize()
{
	int maxW = 0, maxH = 0;
	for(auto& tab: tabs)
	{
		tab->calculateSize();
		maxW = std::max(maxW, tab->getTotalWidth());
		maxH = std::max(maxH, tab->getTotalHeight());
	}
	setPreferredWidth(Size::pixels(maxW+sized(4, 8)));
	setPreferredHeight(Size::pixels(maxH+sized(16, 18)));
}

void TabPanel::arrange(int contX, int contY, int contW, int contH)
{
	Widget::arrange(contX, contY, contW, contH);

	int tabX, tabY, tabW, tabH;
	if(is_large)
	{
		tabX = x+4;
		tabY = y+22;
		tabW = getWidth()-8;
		tabH = getHeight()-18;
	}
	else
	{
		tabX = x+2;
		tabY = y+18;
		tabW = getWidth()-4;
		tabH = getHeight()-16;
	}

	for(auto& tab: tabs)
		tab->arrange(tabX, tabY, tabW, tabH);
}

void TabPanel::realize(DialogRunner& runner)
{
	alDialog = runner.push(shared_from_this(), DIALOG {
		jwin_tab_proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		0, 0,
		nullptr, sized((void*)nullptr, (void*)lfont_l), nullptr
	});

	// jwin_tab_proc needs a pointer to the DIALOG array in dp3...
	runner.dialogConstructed.addListener([this, &runner]() {
		this->alDialog->dp3 = runner.getDialogArray();
	});

	// Realize tab contents and make the first tab active.  We also
	// need to build a TABPANEL array to track what's in each tab.
	jwinTabs.reserve(tabs.size()+1);
	bool first = true;
	for(auto& tab: tabs)
	{
		tab->realize(runner);
		jwinTabs.push_back(TABPANEL {
			tab->getTitle(),
			first ? D_SELECTED : 0,
			tab->getContentIndices(),
			0,
			nullptr
		});
		first = false;
	}

	jwinTabs.push_back(TABPANEL {
		nullptr,
		0,
		nullptr,
		0,
		nullptr
	});

	alDialog->dp = jwinTabs.data();
}

}
