#include "pickruletemplate.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "zquest.h"
#include "zq_files.h"
#include "info.h"

void call_ruletemplate_dlg()
{
	PickRuleTemplateDialog(applyRuleTemplate).show();
}

static const GUI::ListData ruletemplatesList
{
	{ "Bugfix", ruletemplateCompat,
		"Removes all 'Compat' rules" },
	{ "ZScript Bugfix", ruletemplateZSCompat,
		"Remove ZScript compatibility rules" }
};

PickRuleTemplateDialog::PickRuleTemplateDialog(std::function<void(int32_t)> setRuleTemplate):
	setRuleTemplate(setRuleTemplate)
{}

std::shared_ptr<GUI::Widget> PickRuleTemplateDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> cboxes = Rows<2>();
	for(size_t q = 0; q < ruletemplatesList.size(); ++q)
	{
		std::string infostr = ruletemplatesList.getInfo(q);
		cboxes->add(Button(forceFitH = true, text = "?",
			disabled = (infostr.size() < 1),
			onPressFunc = [infostr]()
			{
				InfoDialog("Info",infostr).show();
			}));
		cboxes->add(templates[q] = Checkbox(
			hAlign = 0.0,
			checked = false,
			text = ruletemplatesList.getText(q)
			));
	}
	
	return Window(
		title = "Pick Rule Template",
		onClose = message::CANCEL,
		Column(
			Label
			(
				textAlign = 1,
				text = "Specify which templates you would like to apply."
			),
			cboxes,
			Label(
				hAlign = 0.5,
				maxLines = 2,
				textAlign = 1,
				text = "After creation, you can toggle individual Rules from\n"
					   "'Quest->Options->Rules' and 'ZScript->Quest Script Settings'"
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
}

bool PickRuleTemplateDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		//Exiting messages
		case message::OK:
			for(size_t q = 0; q < ruletemplatesList.size(); ++q)
			{
				if(templates[q]->getChecked())
					setRuleTemplate(ruletemplatesList.getValue(q));
			}
			return true;
		case message::CANCEL:
		default:
			return true;
	}
}
