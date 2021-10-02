#include "qrpanel.h"
#include "grid.h"
#include "checkbox_qr.h"
#include "../zc_alleg.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zquest.h"
#include "../zsys.h"
#include <utility>

#define FONT sized(nfont, lfont_l)


namespace GUI
{

QRPanel::QRPanel(): TabPanel(), message(-1), init_qrs(NULL), qrCount(16)
{}

void QRPanel::loadQRs(byte const* qrs)
{
	init_qrs = qrs;
}

void QRPanel::setCount(size_t count)
{
	qrCount = count;
}

void QRPanel::loadList(GUI::ListData qrlist)
{
	assert(init_qrs);
	size_t q = 0;
	int tabnum = 1;
	if(qrlist.size() <= qrCount+2) //If it can fit without a tabpanel, allow it to
		qrCount+=2;
	while(q < qrlist.size())
	{
		std::shared_ptr<Grid> content = Grid::rows(1);
		for(size_t ind = 0; ind < qrCount; ++ind)
		{
			std::shared_ptr<QRCheckbox> cbox = std::make_shared<QRCheckbox>();
			if(q >= qrlist.size())
			{
				cbox->setText("--");
				cbox->setDisabled(true);
			}
			else
			{
				int qr = qrlist.getValue(q);
				std::string const& name = qrlist.getText(q);
				cbox->setText(name);
				cbox->onToggle(message);
				cbox->setQR(qr);
				if(get_bit(init_qrs, qr))
					cbox->setChecked(true);
				++q;
			}
			cbox->setHAlign(0.0);
			cbox->setHMargins(0_px);
			cbox->setVMargins(1_px);
			content->add(cbox);
		}
		content->setHAlign(0.0);
		content->setVAlign(0.0);
		content->setVMargins(0_px);
		std::shared_ptr<TabRef> tab = std::make_shared<TabRef>();
		tab->setContent(content);
		tab->setName(" "+std::to_string(tabnum++)+" ");
		add(tab);
	}
}

}
