#ifndef ZC_DIALOG_COMBOWIZARD_H
#define ZC_DIALOG_COMBOWIZARD_H

#include "comboeditor.h"
#include <gui/switcher.h>
#include <gui/drop_down_list.h>
#include <gui/radioset.h>
bool hasComboWizard(int32_t type);
void call_combo_wizard(ComboEditorDialog& dlg);
class ComboWizardDialog: public GUI::Dialog<ComboWizardDialog>
{
public:
	enum class message { OK, CANCEL, UPDATE,
		RSET0,RSET1,RSET2,RSET3,RSET4,RSET5,RSET6,RSET7,RSET8,RSET9 };
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	newcombo local_ref;
	newcombo& dest_ref;
	newcombo const& src_ref;
	ComboEditorDialog const& parent;
	std::string thelp, ctyname;
	
	int32_t flags;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tfs[10];
	std::shared_ptr<GUI::DropDownList> ddls[10];
	std::shared_ptr<GUI::Checkbox> cboxes[10];
	std::shared_ptr<GUI::Switcher> switcher[10];
	std::shared_ptr<GUI::CornerSwatch> cswatchs[3];
	std::shared_ptr<GUI::Radio> rset[10][10];
	size_t rs_sz[10];
	
	GUI::ListData lists[10];
	
	GUI::ListData list_lwscript, list_ewscript, list_sprites;
	
	void setRadio(size_t rs, size_t ind);
	size_t getRadio(size_t rs);
	
	void update(bool first = false);
	void endUpdate();
	
	void updateTitle();
	
	ComboWizardDialog(ComboEditorDialog& parent);
	friend void call_combo_wizard(ComboEditorDialog& dlg);
};

#endif
