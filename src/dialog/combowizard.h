#ifndef ZC_DIALOG_COMBOWIZARD_H_
#define ZC_DIALOG_COMBOWIZARD_H_

#include "comboeditor.h"
#include <gui/switcher.h>
#include <gui/frame.h>
#include <gui/tabpanel.h>
#include <gui/drop_down_list.h>
#include <gui/radioset.h>
bool hasComboWizard(int32_t type);
void call_combo_wizard(ComboEditorDialog& dlg);
void combo_default(newcombo& ref, bool typeonly = true);
bool do_combo_default(newcombo& ref);
class ComboWizardDialog: public GUI::Dialog<ComboWizardDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, UPDATE, DEFAULT,
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
	std::shared_ptr<GUI::TabPanel> tpan[10];
	std::shared_ptr<GUI::Button> btns[10];
	std::shared_ptr<GUI::TextField> tfs[10];
	std::shared_ptr<GUI::DropDownList> ddls[10];
	std::shared_ptr<GUI::Checkbox> cboxes[20];
	std::shared_ptr<GUI::Switcher> switcher[10];
	std::shared_ptr<GUI::Label> lbls[10];
	std::shared_ptr<GUI::CornerSwatch> cswatchs[3];
	std::shared_ptr<GUI::SelComboSwatch> cmbswatches[3];
	std::shared_ptr<GUI::Frame> frames[10];
	std::shared_ptr<GUI::Radio> rset[10][10];
	std::shared_ptr<GUI::Grid> grids[10];
	size_t rs_sz[10];
	
	GUI::ListData lists[10];
	
	GUI::ListData list_lwscript, list_ewscript, list_sprites,
		list_dropsets, list_sfx, list_counters,
		list_dirs, list_torch_shapes;
	
	std::map<byte, newcombo> alt_refs;
	
	void setRadio(size_t rs, size_t ind);
	size_t getRadio(size_t rs);
	
	void update(bool first = false);
	void endUpdate();
	
	void updateTitle();
	
	ComboWizardDialog(ComboEditorDialog& parent);
	friend void call_combo_wizard(ComboEditorDialog& dlg);
};

#endif
