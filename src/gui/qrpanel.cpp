#include "qrpanel.h"
#include "grid.h"
#include "checkbox_qr.h"
#include "../zc_alleg.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "zquest.h"
#include "../zsys.h"
#include "dialog/info.h"
#include <utility>


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
	int32_t tabnum = 1;
	if(qrlist.size() <= qrCount+1) //If it can fit without a tabpanel, allow it to
		++qrCount;
	while(q < qrlist.size())
	{
		std::shared_ptr<Grid> content = Grid::rows(2);
		for(size_t ind = 0; ind < qrCount; ++ind)
		{
			std::shared_ptr<QRCheckbox> cbox = std::make_shared<QRCheckbox>();
			std::shared_ptr<Button> ibtn = std::make_shared<Button>();
			ibtn->setText("?");
			if(q >= qrlist.size())
			{
				cbox->setText("--");
				cbox->setDisabled(true);
				ibtn->setDisabled(true);
			}
			else
			{
				int32_t qr = qrlist.getValue(q);
				std::string const& name = qrlist.getText(q);
				std::string const& infotext = qrlist.getInfo(q);
				cbox->setText(name);
				if(infotext.size())
					ibtn->setOnPress([infotext](){InfoDialog("QR Info",infotext).show();});
				else ibtn->setDisabled(true);
				cbox->onToggle(message);
				cbox->setQR(qr);
				if(get_bit(init_qrs, qr))
					cbox->setChecked(true);
				++q;
			}
			cbox->setHAlign(0.0);
			ibtn->setHAlign(1.0);
			cbox->setRightPadding(0_px);
			cbox->setVPadding(2_px);
			ibtn->setPadding(0_px);
			ibtn->overrideWidth(2_em);
			ibtn->setForceFitHei(true); //fit the height of the cbox
			content->add(ibtn);
			content->add(cbox);
		}
		content->setHAlign(0.0);
		content->setVAlign(0.0);
		std::shared_ptr<TabRef> tab = std::make_shared<TabRef>();
		tab->setContent(content);
		tab->setName(" "+std::to_string(tabnum++)+" ");
		add(tab);
	}
}

}
