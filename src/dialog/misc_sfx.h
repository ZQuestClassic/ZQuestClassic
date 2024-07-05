#ifndef ZC_DIALOG_MISCSFX_H_
#define ZC_DIALOG_MISCSFX_H_

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "base/misctypes.h"

class MiscSFXDialog: public GUI::Dialog<MiscSFXDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	MiscSFXDialog(byte* vals, size_t vals_per_tab, std::function<void(int32_t*)> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	GUI::ListData sfx_list;
	std::function<void(int32_t*)> setVals;
	int32_t local_sfx[sfxMAX];
	size_t vals_per_tab;
};

#endif

