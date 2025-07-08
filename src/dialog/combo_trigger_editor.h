#ifndef ZC_DIALOG_COMBOTRIGGEREDITOR_H_
#define ZC_DIALOG_COMBOTRIGGEREDITOR_H_

#include "comboeditor.h"
#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <zq/gui/seltile_swatch.h>
#include <zq/gui/selcombo_swatch.h>
#include <zq/gui/tileanim_frame.h>
#include <zq/gui/cornerselect.h>
#include <functional>
#include <string_view>
#include <map>
#include "base/combo.h"

bool call_trigger_editor(ComboEditorDialog& dlg, size_t index);

class ComboTriggerDialog: public GUI::Dialog<ComboTriggerDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, CLEAR, WARNINGS, PLUSTRIGGER, MINUSTRIGGER };

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	combo_trigger local_ref;
	newcombo& parent_comboref;
	size_t index;
	ComboEditorDialog const& parent;
	vector<string> warnings;
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> l_minmax_trig;
	std::shared_ptr<GUI::TextField> req_litems_field, trig_buttons_field, spawned_ip_field;
	std::shared_ptr<GUI::Button> warnbtn;
	
	ComboTriggerDialog(ComboEditorDialog& parentdlg, combo_trigger& trigger, size_t index);
	
	void updateWarnings();
	bool apply_trigger();
	
	std::shared_ptr<GUI::Checkbox> TRIGFLAG(int index, const char* str, int cspan = 1, bool right = false);
	friend bool call_trigger_editor(ComboEditorDialog& dlg, size_t index);
	friend class ComboWizardDialog;
};

#endif
