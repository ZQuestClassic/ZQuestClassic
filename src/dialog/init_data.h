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
	enum class message { OK, CANCEL, LEVEL };

	InitDataDialog(zinitdata const& start, std::function<void(zinitdata const&)> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(message msg);

private:
	void setOfs(size_t ofs);
	std::shared_ptr<GUI::TextField> sBombMax;
	std::shared_ptr<GUI::Label> l_lab[10];
	std::shared_ptr<GUI::Checkbox> l_maps[10];
	std::shared_ptr<GUI::Checkbox> l_comp[10];
	std::shared_ptr<GUI::Checkbox> l_bkey[10];
	std::shared_ptr<GUI::TextField> l_keys[10];
	zinitdata local_zinit;
	size_t levelsOffset;
	std::function<void(zinitdata const&)> setVals;
};

#endif
