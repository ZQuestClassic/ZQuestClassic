#ifndef ZC_DIALOG_PICKRULESET_H_
#define ZC_DIALOG_PICKRULESET_H_

#include <gui/dialog.h>
#include "gui/radioset.h"
#include "gui/label.h"
#include <functional>
#include <string_view>

bool call_ruleset_dlg(byte* dest_qrs = nullptr);

class PickRulesetDialog: public GUI::Dialog<PickRulesetDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RULESET };

	PickRulesetDialog(std::function<void(int32_t,byte*)> setRuleset, byte* dest_qrs);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	byte* dest_qrs;
	std::shared_ptr<GUI::RadioSet> rulesetChoice;
	std::shared_ptr<GUI::Label> rulesetInfo;
	std::function<void(int32_t,byte*)> setRuleset;
};

#endif
