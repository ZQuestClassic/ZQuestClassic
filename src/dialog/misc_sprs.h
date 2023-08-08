#ifndef ZC_DIALOG_MISCSPRS_H
#define ZC_DIALOG_MISCSPRS_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "base/misctypes.h"

class MiscSprsDialog: public GUI::Dialog<MiscSprsDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	MiscSprsDialog(byte* vals, size_t vals_per_tab, std::function<void(int32_t*)> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	GUI::ListData sprs_list;
	std::function<void(int32_t*)> setVals;
	int32_t local_sprs[sprMAX];
	size_t vals_per_tab;
};

#endif

