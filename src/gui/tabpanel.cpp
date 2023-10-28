#include "gui/tabpanel.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include <utility>

namespace GUI
{

TabPanel::TabPanel(): indexptr(NULL), visibleChild(0), onSwitch(NULL)
{
	setFitParent(true);
}

void TabPanel::switchTo(size_t index)
{
	if(!alDialog) //Not realized yet?
	{
		visibleChild = index;
		return;
	}
	if(index == visibleChild)
		return;

	assert(index<children.size());
	
	if(indexptr) *indexptr = index;
	if(onSwitch)
		onSwitch(visibleChild, index);
	children[visibleChild]->setExposed(false); //Hide the old child
	visibleChild = index;
	pendDraw(); //Draw the tabpanel again, with the correct tab selected
	children[index]->setExposed(true); //Show the new child
}
void TabPanel::setDisabled(size_t index, bool newdis)
{
	if(index >= children.size())
		return;
	children.at(index)->setDisabled(newdis);
	//Select a new tab if current tab is disabled?
	if(index == visibleChild && newdis)
	{
		auto oldind = index;
		do
		{
			++index;
			if(index >= children.size())
				index = 0;
		}
		while(index != oldind && getDisabled(index));
		if(index != oldind)
			switchTo(index);
	}
}
bool TabPanel::getDisabled(size_t index) const
{
	if(index >= children.size())
		return true;
	return children.at(index)->getDisabled();
}

void TabPanel::setPtr(size_t* ptr)
{
	indexptr = ptr;
	if(ptr)
		switchTo(*ptr);
}

void TabPanel::add(std::shared_ptr<Widget> child)
{
	if(std::shared_ptr<TabRef> tbchld = std::dynamic_pointer_cast<TabRef>(child))
	{
		if(alDialog && //Only do this if dialog is already realized?
			children.size() != visibleChild)
			tbchld->setExposed(false);
		children.emplace_back(std::move(tbchld));
	}
}

void TabPanel::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	// We'll handle invisibility by applying an extra "hide" to each child.
	// That's easier than keeping precise track of what's visible or not.
	if(alDialog) alDialog.applyVisibility(visible);
	for(auto& child: children)
		child->setExposed(visible);
}

void TabPanel::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
	for(auto& child: children)
		child->setDisabled(dis);
}

void TabPanel::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

void TabPanel::calculateSize()
{
	if(children.size() == 1)
	{
		auto& child = children.at(0);
		child->calculateSize();
		setPreferredWidth(Size::pixels(child->getTotalWidth()));
		setPreferredHeight(Size::pixels(child->getTotalHeight()));
		setPadding(0_px);
	}
	else
	{
		// The switcher is as large as its largest child, or large enough to fit its' tabs without using arrows
		int32_t maxW = 64, maxH = 0, tabwid = 0;
		for(auto& child: children)
		{
			child->calculateSize();
			tabwid+=text_length(widgFont, child->getName().c_str())+15;
			int32_t w = child->getTotalWidth(); // Should this be getTotalWidth()?
			if(w > maxW)
				maxW = w;
			int32_t h = child->getTotalHeight(); // getTotalHeight()?
			if(h > maxH)
				maxH = h;
		}
		if(tabwid > maxW)
			maxW = tabwid;
		setPreferredWidth(Size::pixels(maxW+6));
		setPreferredHeight(Size::pixels(text_height(widgFont) + 4) + Size::pixels(maxH));
	}
	Widget::calculateSize();
}

void TabPanel::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	if(children.size() == 1)
	{
		Widget::arrange(contX, contY, contW, contH);
		auto& child = children.at(0);
		child->arrange(contX, contY, contW, contH);
	}
	else for(auto& child: children)
	{
		Widget::arrange(contX, contY, contW, contH);
		child->arrange(x, y+text_height(widgFont)+9, getWidth(), getHeight()-(text_height(widgFont)+9));
	}
}

void TabPanel::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	if(children.size() == 1)
	{
		auto& child = children.at(0);
		child->realize(runner);
		return;
	}
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<new_tab_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags,
		0, 0, // d1, d2
		this, widgFont, nullptr // dp, dp2, dp3
	});
	
	for(auto& child: children)
	{
		child->realize(runner);
		child->setExposed(false);
	}
	if(visibleChild >= children.size())
		visibleChild = 0;
	if(indexptr) *indexptr = visibleChild;
	children[visibleChild]->setExposed(true);
}

int32_t TabPanel::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	
	return -1;
}
void TabPanel::setOnSwitch(std::function<void(size_t,size_t)> newOnSwitch)
{
	onSwitch = std::move(newOnSwitch);
}

}
