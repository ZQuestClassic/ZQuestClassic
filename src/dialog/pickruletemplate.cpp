#include "pickruletemplate.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/zq_files.h"
#include "info.h"
#include "zc_list_data.h"

static bool modified;
bool call_ruletemplate_dlg(byte* dest)
{
	modified = false;
	PickRuleTemplateDialog(dest).show();
	return modified;
}

PickRuleTemplateDialog::PickRuleTemplateDialog(byte* dest_qrs) :
	dest_qrs(dest_qrs),
	list_rule_templates(GUI::ZCListData::rule_templates_list())
{}

std::shared_ptr<GUI::Widget> PickRuleTemplateDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> cboxes = Rows<2>();
	for(size_t q = 0; q < list_rule_templates.size(); ++q)
	{
		std::string infostr = list_rule_templates.getInfo(q);
		cboxes->add(Button(forceFitH = true, text = "?",
			disabled = (infostr.size() < 1),
			onPressFunc = [infostr]()
			{
				InfoDialog("Info",infostr).show();
			}));
		cboxes->add(templates[q] = Checkbox(
			hAlign = 0.0,
			checked = false,
			text = list_rule_templates.getText(q)
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
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
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
			for(size_t q = 0; q < list_rule_templates.size(); ++q)
			{
				if(templates[q]->getChecked())
				{
					modified = true;
					applyRuleTemplateWithConfirmation(list_rule_templates.getValue(q),dest_qrs);
				}
			}
			return modified;
		case message::CANCEL:
			return true;
	}
	return false;
}
