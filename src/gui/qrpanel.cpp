#include "gui/qrpanel.h"
#include "gui/grid.h"
#include "gui/scrolling_pane.h"
#include "gui/checkbox_qr.h"
#include "gui/label.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include "dialog/info.h"
#include <utility>

std::string const& getTagName(int32_t ruletype);
std::string const& getLongestTagName();

namespace GUI
{

QRPanel::QRPanel(): TabPanel(), scrolling(false), showTags(false),
	indexed(false), scrollWidth(0_px), scrollHeight(0_px), message(-1),
	info_message(-1), local_qrs(NULL), qrCount(16)
{}

void QRPanel::loadQRs(byte* qrs)
{
	local_qrs = qrs;
}

void QRPanel::setCount(size_t count)
{
	qrCount = count;
}
void QRPanel::setShowTags(bool v)
{
	showTags = v;
}
void QRPanel::setIndexed(bool v)
{
	indexed = v;
}

void QRPanel::setScrollWidth(Size sz)
{
	scrollWidth = sz;
}
void QRPanel::setScrollHeight(Size sz)
{
	scrollHeight = sz;
}

void QRPanel::loadList(GUI::ListData qrlist)
{
	assert(local_qrs);
	size_t q = 0;
	int32_t tabnum = 1;
	scrolling = qrCount == 0;
	if(scrolling)
		qrCount = qrlist.size();
	else if(qrlist.size() <= qrCount+1) //If it can fit without a tabpanel, allow it to
		++qrCount;
	
	if(!qrlist.size())
	{
		std::shared_ptr<TabRef> tab = std::make_shared<TabRef>();
		if(scrolling)
		{
			scrollpane = std::make_shared<ScrollingPane>();
			scrollpane->setContent(std::make_shared<DummyWidget>());
			scrollpane->setPadding(0_px);
			if(scrollHeight > 0)
				scrollpane->minHeight(scrollHeight);
			if(scrollWidth > 0)
				scrollpane->minWidth(scrollWidth);
			tab->setContent(scrollpane);
			tab->setName(" ");
		}
		else
		{
			tab->setContent(std::make_shared<DummyWidget>());
			tab->setName(" ");
		}
		add(tab);
		return;
	}
	while(q < qrlist.size())
	{
		std::shared_ptr<Grid> content = Grid::rows((scrolling&&showTags)?3:2);
		for(size_t ind = 0; ind < qrCount; ++ind)
		{
			std::shared_ptr<QRCheckbox> cbox = std::make_shared<QRCheckbox>();
			std::shared_ptr<Button> ibtn = std::make_shared<Button>();
			ibtn->setText("?");
			ListItem* li = nullptr;
			if(q >= qrlist.size())
			{
				cbox->setText("--");
				cbox->setDisabled(true);
				ibtn->setDisabled(true);
			}
			else if((li = &(qrlist.accessIndex(q))))
			{
				int32_t qr = indexed ? q : li->value;
				std::string const& name = li->text;
				std::string const& infotext = li->info;
				cbox->setText(name);
				if(infotext.size())
					ibtn->setOnPress([infotext,this](){InfoDialog("QR Info",infotext,nullopt,local_qrs).show();});
				else ibtn->setDisabled(true);
				cbox->onToggle(message);
				cbox->setQR(qr);
				if(get_bit(local_qrs, qr))
					cbox->setChecked(true);
				++q;
			}
			cbox->setHAlign(0.0);
			ibtn->setHAlign(1.0);
			ibtn->onClick(info_message);
			cbox->setRightPadding(0_px);
			cbox->setVPadding(2_px);
			ibtn->setPadding(0_px);
			ibtn->overrideWidth(2_em);
			ibtn->setForceFitHei(true); //fit the height of the cbox
			content->add(ibtn);
			content->add(cbox);
			if(scrolling && showTags)
			{
				//ASSERT(li);
				std::shared_ptr<Label> lbl = std::make_shared<Label>();
				lbl->setHAlign(1.0);
				content->add(lbl);
				
				if(scrollWidth > 0_px) //width forcing
				{
					//Calculate the width
					Size cbwid = scrollWidth;
					//Calculate the longest possible width
					lbl->setText(getLongestTagName());
					lbl->calculateSize();
					lbl->setAlign(2);
					lbl->setHAlign(0.0);
					cbwid -= lbl->getTotalWidth();
					lbl->minWidth(Size::pixels(lbl->getWidth()));
					//Set the real text back
					lbl->setText(getTagName(li->tag));
					
					//..and the button width
					ibtn->calculateSize();
					cbwid -= ibtn->getTotalWidth();
					
					//..and the padding on the cbox
					cbox->calculateSize();
					cbwid -= cbox->getTotalWidth()-cbox->getWidth();
					
					//Force the cbox width
					cbox->overrideWidth(cbwid);
				}
				else
				{
					lbl->setText(getTagName(li->tag));
				}
			}
		}
		
		std::shared_ptr<TabRef> tab = std::make_shared<TabRef>();
		
		content->setVAlign(0.0);
		if(scrolling)
		{
			scrollpane = std::make_shared<ScrollingPane>();
			scrollpane->setContent(content);
			scrollpane->setPtr((int32_t*)indexptr);
			scrollpane->setPadding(0_px);
			if(scrollHeight > 0)
				scrollpane->minHeight(scrollHeight);
			if(scrollWidth > 0)
				scrollpane->minWidth(scrollWidth);
			
			tab->setContent(scrollpane);
			tab->setName(" ");
		}
		else
		{
			content->setHAlign(0.0);
			tab->setContent(content);
			tab->setName(" "+std::to_string(tabnum++)+" ");
		}
		add(tab);
	}
}
ScrollingPane* QRPanel::getScrollPane()
{
	if (scrolling)
	{
		return scrollpane.get();
	}
	return nullptr;
}

}

