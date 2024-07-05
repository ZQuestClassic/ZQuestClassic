#ifndef ZC_DIALOG_CHEATCODES_H_
#define ZC_DIALOG_CHEATCODES_H_

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
	enum class message { REFR_INFO, CREATE_INIT_DATA, EDIT_INIT_DATA, OK, CANCEL };

	TestQstDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	GUI::ListData dmap_list;
};

#endif
