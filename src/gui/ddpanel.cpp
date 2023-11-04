#include "gui/ddpanel.h"
#include "gui/grid.h"
#include "gui/drop_down_list.h"
#include "gui/label.h"
#include "base/zc_alleg.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include "dialog/info.h"
#include <utility>


namespace GUI
{

DDPanel::DDPanel(): TabPanel(), message(-1), theVals(NULL), dataList(NULL),
	ddCount(16)
{}

void DDPanel::linkVals(int32_t* vals)
{
	theVals = vals;
}

void DDPanel::setCount(size_t count)
{
	ddCount = count;
}

void DDPanel::loadDDList(GUI::ListData const& theData)
{
	dataList = &theData;
}

void DDPanel::loadList(GUI::ListData dd_data)
{
	assert(theVals);
	size_t q = 0;
	int32_t tabnum = 1;
	if(dd_data.size() <= ddCount+2) //If it can fit without a tabpanel, allow it to
		ddCount+=2;
	while(q < dd_data.size())
	{
		std::shared_ptr<Grid> content = Grid::rows(3);
		for(size_t ind = 0; ind < ddCount; ++ind)
		{
			if(q >= dd_data.size())
				break;
			std::shared_ptr<DropDownList> ddown = std::make_shared<DropDownList>();
			std::shared_ptr<Button> ibtn = std::make_shared<Button>();
			std::shared_ptr<Label> ddlabel = std::make_shared<Label>();
			ibtn->setText("?");
			
			int32_t indexVal = dd_data.getValue(q);
			std::string const& name = dd_data.getText(q);
			std::string const& infotext = dd_data.getInfo(q);
			ddlabel->setText(name);
			if(infotext.size())
				ibtn->setOnPress([infotext](){InfoDialog("Info",infotext).show();});
			else ibtn->setDisabled(true);
			ddown->setListData(*dataList);
			ddown->setSelectedValue(theVals[q]);
			ddown->setOnSelectFunc([&,q](int32_t val)
				{
					theVals[q] = val;
				});
			ddown->onSelectionChanged(message);
			++q;
			
			ddlabel->setHAlign(1.0);
			ddown->setHAlign(1.0);
			ibtn->setHAlign(0.0);
			ddlabel->setPadding(0_px);
			ddown->setPadding(0_px);
			ibtn->setPadding(0_px);
			ibtn->overrideWidth(2_em);
			ibtn->setForceFitHei(true); //fit the height of the row
			content->add(ddlabel);
			content->add(ddown);
			content->add(ibtn);
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
