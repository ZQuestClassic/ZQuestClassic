#ifndef ZC_DIALOG_COMBOWIZARD_H
#define ZC_DIALOG_COMBOWIZARD_H

#include "comboeditor.h"
bool hasComboWizard(int32_t type);
void call_combo_wizard(ComboEditorDialog& dlg);
class ComboWizardDialog: public GUI::Dialog<ComboWizardDialog>
{
public:
	enum class message { OK, CANCEL };
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	newcombo local_ref;
	ComboEditorDialog& parent;
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tfs[10];
	
	void update();
	
	ComboWizardDialog(ComboEditorDialog& parent) : parent(parent),
		local_ref(parent.local_comboref)
	{}
	friend void call_combo_wizard(ComboEditorDialog& dlg);
};

#endif
