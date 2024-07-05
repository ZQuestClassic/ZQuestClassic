#ifndef ZC_DIALOG_SCRIPTRULES_H_
#define ZC_DIALOG_SCRIPTRULES_H_

#include "base/qrs.h"
#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>

#define QR_SZ QUESTRULES_NEW_SIZE

class ScriptRulesDialog: public GUI::Dialog<ScriptRulesDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TOGGLE_QR };

	ScriptRulesDialog(byte const* qrs, size_t qrs_per_tab, std::function<void(byte*)> setQRs);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::function<void(byte*)> setQRs;
	byte local_qrs[QR_SZ];
	size_t qrs_per_tab;
};

#endif

