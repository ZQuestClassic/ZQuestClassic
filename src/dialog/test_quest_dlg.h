#ifndef ZC_DIALOG_CHEATCODES_H
#define ZC_DIALOG_CHEATCODES_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_testqst_dialog();

/* Note: The string_views passed to setCheatCodes will include
 * null terminators.
 */
class TestQstDialog: public GUI::Dialog<TestQstDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	TestQstDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	GUI::ListData dmap_list;
};

#endif
