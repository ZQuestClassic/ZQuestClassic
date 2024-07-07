#ifndef ZC_DIALOG_COMBOPOOL_H_
#define ZC_DIALOG_COMBOPOOL_H_

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <zq/gui/selcombo_swatch.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>

void call_cpool_dlg(int32_t index);

struct cpool_widg
{
	// std::shared_ptr<GUI::Button> xbtn;
	// std::shared_ptr<GUI::Button> left_btn;
	// std::shared_ptr<GUI::Button> right_btn;
	// std::shared_ptr<GUI::Button> plus_btn;
	// std::shared_ptr<GUI::Button> minus_btn;
	std::shared_ptr<GUI::TextField> weight_txt;
	std::shared_ptr<GUI::Checkbox> use_cs;
	std::shared_ptr<GUI::SelComboSwatch> cpane;
};

class ComboPoolDialog: public GUI::Dialog<ComboPoolDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD
	};

	ComboPoolDialog(){}

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::vector<cpool_widg> widgs;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Button> pastebtn;
};

#endif
