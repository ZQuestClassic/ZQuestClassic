#ifndef ZC_DIALOG_PICKRULETMP_H
#define ZC_DIALOG_PICKRULETMP_H

#include <gui/dialog.h>
#include "gui/checkbox.h"
#include "gui/label.h"
#include "base/qrs.h"
#include <functional>
#include <string_view>
#include "zq/zq_files.h"

void call_ruletemplate_dlg();
extern const GUI::ListData ruletemplatesList;

class PickRuleTemplateDialog: public GUI::Dialog<PickRuleTemplateDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	PickRuleTemplateDialog(std::function<void(int32_t, byte*)> setRuleTemplate);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Checkbox> templates[sz_ruletemplate];
	std::function<void(int32_t,byte*)> setRuleTemplate;
};

#endif
