#ifndef ZC_DIALOG_ITEMWIZARD_H
#define ZC_DIALOG_ITEMWIZARD_H

#include "itemeditor.h"
#include <gui/switcher.h>
#include <gui/frame.h>
#include <gui/tabpanel.h>
#include <gui/drop_down_list.h>
#include <gui/radioset.h>
bool hasItemWizard(int32_t type);
void call_item_wizard(ItemEditorDialog& dlg);
void item_default(itemdata& ref);
bool do_item_default(itemdata& ref);
class ItemWizardDialog: public GUI::Dialog<ItemWizardDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, UPDATE, DEFAULT,
		RSET0,RSET1,RSET2,RSET3,RSET4,RSET5,RSET6,RSET7,RSET8,RSET9 };
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	itemdata local_ref;
	itemdata& dest_ref;
	itemdata const& src_ref;
	ItemEditorDialog const& parent;
	std::string thelp, ityname;
	
	int32_t flags;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TabPanel> tpan[10];
	std::shared_ptr<GUI::Button> btns[10];
	std::shared_ptr<GUI::TextField> tfs[10];
	std::shared_ptr<GUI::DropDownList> ddls[10];
	std::shared_ptr<GUI::Checkbox> cboxes[20];
	std::shared_ptr<GUI::Switcher> switcher[10];
	std::shared_ptr<GUI::Label> lbls[10];
	std::shared_ptr<GUI::Frame> frames[10];
	std::shared_ptr<GUI::Radio> rset[10][10];
	std::shared_ptr<GUI::Grid> grids[10];
	size_t rs_sz[10];
	
	GUI::ListData lists[10];
	
	GUI::ListData list_sfx;
	
	void setRadio(size_t rs, size_t ind);
	size_t getRadio(size_t rs);
	
	void update(bool first = false);
	void endUpdate();
	
	void updateTitle();
	
	map<uint,vector<std::shared_ptr<GUI::Widget>>> widgs;
	std::shared_ptr<GUI::Widget> push_widg(uint id, std::shared_ptr<GUI::Widget> widg);
	void disable(uint id, bool dis);
	
	std::shared_ptr<GUI::Widget> CBOX_IMPL(string const& name, int32_t* mem, int32_t bit, optional<string> info = nullopt);
	ItemWizardDialog(ItemEditorDialog& parent);
	friend void call_item_wizard(ItemEditorDialog& dlg);
};

#endif
