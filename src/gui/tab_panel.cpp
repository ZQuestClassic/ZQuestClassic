#include "tab_panel.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <algorithm>
#include <utility>

#define FONT sized(nfont, lfont_l)

namespace GUI
{

// This should all be cleaned up and optimized a bit more, but maybe later.

// Reimplemented from tab_ctl.cpp.
int TabPanel::clickedTab(int firstTab, int x)
{
    int width=0;

    for(int i = firstTab; i < tabs.size(); ++i)
    {
        width += text_length(FONT, tabs[i]->getTitle())+15;
        if(width > x)
            return i;
    }

    return -1;
}

// Reimplemented from tab_ctl.cpp
int TabPanel::visibleTabsWidth(int firstTab, int maxWidth)
{
    int total = 0;

	int lastTab=lastVisibleTab(firstTab, maxWidth);
	for(int i = firstTab; i <= lastTab; ++i)
		total+=text_length(FONT, tabs[i]->getTitle())+15;

    return total+1;
}

// Reimplemented from tab_ctl.cpp
int TabPanel::tabsWidth()
{
    int total = 0;
    for(size_t i = 0; i < tabs.size(); ++i)
        total += text_length(FONT, tabs[i]->getTitle())+15;
    return total+1;
}

// Reimplemented from tab_ctl.cpp
bool TabPanel::usesTabArrows(int maxWidth)
{
    return tabsWidth() > maxWidth;
}

// Reimplemented from tab_ctl.cpp
int TabPanel::lastVisibleTab(int firstTab, int maxWidth)
{
    int i=0;
    int w=0;

    if(usesTabArrows(maxWidth))
        maxWidth-=30;

    for(i = firstTab; i < tabs.size(); ++i)
    {
        w += text_length(FONT, tabs[i]->getTitle())+15;
        if(w > maxWidth)
            return i-1;
    }

    return i-1;
}

// Based on jwin_tab_proc.
int TabPanel::proc(int msg, DIALOG *d, int c)
{
	int tx;
	int sd = 2;

	switch(msg)
	{
	case MSG_DRAW:
	{
		TabPanel& tp = *static_cast<TabPanel*>(d->dp);
		auto* screen = gui_get_screen();

		int textHeight=text_height(FONT);

        rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+8+textHeight, scheme[jcBOX]); //tab area
        rectfill(screen, d->x+1, d->y+sd+textHeight+7, d->x+d->w-2, d->y+sd+d->h-2, scheme[jcBOX]); //panel
        _allegro_vline(screen, d->x, d->y+sd+7+textHeight, d->y+sd+d->h-2, scheme[jcLIGHT]);
        _allegro_vline(screen, d->x+1, d->y+sd+7+textHeight, d->y+sd+d->h-3, scheme[jcMEDLT]);
        _allegro_vline(screen, d->x+d->w-2, d->y+sd+7+textHeight, d->y+sd+d->h-2, scheme[jcMEDDARK]);
        _allegro_vline(screen, d->x+d->w-1, d->y+sd+7+textHeight-1, d->y+sd+d->h-1, scheme[jcDARK]);
        _allegro_hline(screen, d->x+1, d->y+sd+d->h-2, d->x+d->w-3, scheme[jcMEDDARK]);
        _allegro_hline(screen, d->x, d->y+sd+d->h-1, d->x+d->w-2, scheme[jcDARK]);
        tx = d->x;

		// If the first visible tab is not also selected
        if(tp.selectedTab!=d->d1)
        {
            _allegro_hline(screen, tx+1, d->y+sd+6+textHeight+1, tx+2, scheme[jcMEDLT]); //initial bottom
            _allegro_hline(screen, tx, d->y+sd+6+textHeight, tx+1, scheme[jcLIGHT]);     //initial bottom
        }

        tx+=2;

        for(int i=d->d1; i <= tp.lastVisibleTab(d->d1, d->w); ++i)
        {
			int textLength=text_length(FONT, tp.tabs[i]->getTitle());
            sd = (i == tp.selectedTab) ? 0 : 2;

			// Draw the left side
            if(i == d->d1 || i-1 != tp.selectedTab)
            {
                _allegro_vline(screen, tx-(2-sd), d->y+sd+2, d->y+8+textHeight, scheme[jcLIGHT]);
                _allegro_vline(screen, tx-(2-sd)+1, d->y+sd+2, d->y+8+textHeight, scheme[jcMEDLT]);
                putpixel(screen, tx+1-(2-sd), d->y+sd+1, scheme[jcLIGHT]);
            }

            _allegro_hline(screen, tx+2-(2-sd), d->y+sd, tx+12+(2-sd)+textLength, scheme[jcLIGHT]); //top
            _allegro_hline(screen, tx+2-(2-sd), d->y+sd+1, tx+12+(2-sd)+textLength, scheme[jcMEDLT]); //top

			// Draw the bottom
            if(i != tp.selectedTab)
            {
                _allegro_hline(screen, tx+1, d->y+sd+6+textHeight, tx+13+textLength+1, scheme[jcLIGHT]);
                _allegro_hline(screen, tx, d->y+sd+6+textHeight+1, tx+13+textLength+1, scheme[jcMEDLT]);
            }

            tx += 4;
            gui_textout_ln(screen, FONT, (unsigned char*)tp.tabs[i]->getTitle(), tx+4, d->y+sd+4, scheme[jcBOXFG], scheme[jcBOX], 0);
            tx += textLength+10;

			// Draw the right side
            if(i==tp.tabs.size()-1 || tp.selectedTab!=i+1)
            {
                putpixel(screen, tx-1+(2-sd), d->y+sd+1, scheme[jcDARK]);
                _allegro_vline(screen, tx+(2-sd), d->y+sd+2, d->y+8+text_height(FONT)-1, scheme[jcDARK]);
                _allegro_vline(screen, tx+(2-sd)-1, d->y+sd+2, d->y+8+text_height(FONT)-(sd?1:0), scheme[jcMEDDARK]);
            }

            ++tx;
        }

        if(d->d1!=0 || tp.lastVisibleTab(d->d1, d->w)+1 < tp.tabs.size())
        {
            jwin_draw_text_button(screen,d->x+d->w-14,d->y+2, 14, 14, "\x8B", 0, true);
            jwin_draw_text_button(screen,d->x+d->w-28,d->y+2, 14, 14, "\x8A", 0, true);
        }

        if((tx+(2-sd))<(d->x+d->w))
        {
            _allegro_hline(screen, tx+(2-sd)-1, d->y+8+text_height(FONT), d->x+d->w-1, scheme[jcLIGHT]); //ending bottom
            _allegro_hline(screen, tx+(2-sd)-2, d->y+8+text_height(FONT)+1, d->x+d->w-2, scheme[jcMEDLT]); //ending bottom
        }

		break;
    }
	case MSG_CLICK:
	{
		TabPanel& tp = *static_cast<TabPanel*>(d->dp);

		// Clicked an arrow button?
		if(tp.usesTabArrows(d->w)
		&& mouse_in_rect(d->x+d->w-28, d->y+2, 28, 14))
		{
			if(mouse_in_rect(d->x+d->w-28, d->y+2, 14, 14))
			{
				if(do_text_button_reset(d->x+d->w-28, d->y+2, 14, 14, "\x8A"))
				{
					// Arrow left
					if(d->d1 == 0)
						break;
					--d->d1;
					return D_REDRAW;
				}
			}
			else if(mouse_in_rect(d->x+d->w-14, d->y+2, 14, 14))
			{
				if(do_text_button_reset(d->x+d->w-14, d->y+2, 14, 14, "\x8B"))
				{
					// Arrow right
					int oldLastTab = tp.lastVisibleTab(d->d1, d->w);
					if(oldLastTab == tp.tabs.size()-1)
						break;
					do
					{
						++d->d1;
					} while(tp.lastVisibleTab(d->d1, d->w) == oldLastTab);
					return D_REDRAW;
				}
			}
		}
		// Didn't click an arrow button; maybe a tab?
		else if(mouse_in_rect(d->x+2, d->y+2,
			tp.visibleTabsWidth(d->d1, d->w),
			text_height(FONT)+9))
        {
			int selected=tp.clickedTab(d->d1, gui_mouse_x()-d->x-2);
			if(selected >= 0 && selected <= tp.lastVisibleTab(d->d1, d->w))
			{
				tp.selectTab(selected);
				return D_REDRAW;
			}
        }
		break;
	}
	}
	return D_O_K;
}

TabPanel::TabPanel(): selectedTab(0)
{
	fgColor = vc(0);
	bgColor = vc(15);
}

void TabPanel::selectTab(size_t newTab)
{
	if(newTab==selectedTab)
		return;

	tabs[selectedTab]->setExposed(false);
	tabs[newTab]->setExposed(true);
	selectedTab=newTab;
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

	int tabsW=tabsWidth();
	if(tabsW>maxW)
		maxW=tabsW;

	setPreferredWidth(Size::pixels(maxW+sized(4, 8)));
	setPreferredHeight(Size::pixels(maxH+sized(16, 20)));
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
		tabH = getHeight()-20;
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
	assert(selectedTab<tabs.size());

	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		0, 0, // d1: first visible tab
		this, nullptr, nullptr
	});

	// Realize tab contents and make sure only the first tab is visible.
	bool first=true;
	for(auto& tab: tabs)
	{
		tab->realize(runner);
		if(first)
			first=false;
		else
			tab->setExposed(false);
	}
}

}
