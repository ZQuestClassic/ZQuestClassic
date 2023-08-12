#ifndef ZC_DIALOG_PICKRULESET_H
#define ZC_DIALOG_PICKRULESET_H

#include <gui/dialog.h>
#include "gui/radioset.h"
#include "gui/label.h"
#include <functional>
#include <string_view>

void call_ruleset_dlg();

class PickRulesetDialog: public GUI::Dialog<PickRulesetDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RULESET };

	PickRulesetDialog(std::function<void(int32_t)> setRuleset);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::RadioSet> rulesetChoice;
	std::shared_ptr<GUI::Label> rulesetInfo;
	std::function<void(int32_t)> setRuleset;
};

#endif
