#ifndef ZC_DIALOG_INITDLG_H
#define ZC_DIALOG_INITDLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_init_dlg(zinitdata& sourcezinit);

class InitDataDialog: public GUI::Dialog<InitDataDialog>
{
public:
	enum class message { OK, CANCEL };

	InitDataDialog(zinitdata const& start, std::function<void(zinitdata const&)> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(message msg);

private:
	std::shared_ptr<GUI::TextField> sBombMax;
	zinitdata local_zinit;
	std::function<void(zinitdata const&)> setVals;
};

#endif
