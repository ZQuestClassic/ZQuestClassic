#ifndef ZC_DIALOG_COMBOEDITOR_H_
#define ZC_DIALOG_COMBOEDITOR_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/list.h>
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

class ComboWizardDialog;
bool call_combo_editor(int32_t index);

std::string getComboTypeHelpText(int32_t id);
std::string getMapFlagHelpText(int32_t id);
int32_t solidity_to_flag(int32_t val);

class ComboEditorDialog: public GUI::Dialog<ComboEditorDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, COMBOTYPE, COMBOFLAG,
		HFLIP, VFLIP, ROTATE, PLUSCS, MINUSCS, MINUSCOMBO, PLUSCOMBO,
		CLEAR, DEFAULT, TILESEL, WIZARD, WARNINGS };
	

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	ComboEditorDialog(newcombo const& ref, int32_t index);
	ComboEditorDialog(int32_t index);
	
	void refreshScript();
	void loadComboType();
	void loadComboFlag();
	void updateCSet();
	void updateAnimation();
	void updateWarnings();
	void updateTriggerIndex();
	void flipSwatches(int rot, int hflip, int vflip);
	void updateFlip(int nflip);
	void apply_combo();
	void add_combo_trigger(size_t pos, bool copied = false);
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_initd[8];
	std::shared_ptr<GUI::Label> l_initds[8];
	std::shared_ptr<GUI::Label> l_attributes[4];
	std::shared_ptr<GUI::Label> l_attribytes[8];
	std::shared_ptr<GUI::Label> l_attrishorts[8];
	std::shared_ptr<GUI::Checkbox> l_flags[16];
	std::shared_ptr<GUI::Button> ib_initds[8];
	std::shared_ptr<GUI::Button> ib_attributes[4];
	std::shared_ptr<GUI::Button> ib_attribytes[8];
	std::shared_ptr<GUI::Button> ib_attrishorts[8];
	std::shared_ptr<GUI::Button> ib_flags[16];
	std::shared_ptr<GUI::Button> wizardButton, warnbtn;
	std::shared_ptr<GUI::Label> l_flip;
	std::shared_ptr<GUI::Label> l_cset;
	std::shared_ptr<GUI::CornerSwatch> cswatchs[3];
	std::shared_ptr<GUI::SelTileSwatch> tswatch;
	std::shared_ptr<GUI::SelComboSwatch> cycleswatch;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::shared_ptr<GUI::List> triggerList;
	std::shared_ptr<GUI::Button> trigbtnAddCursor, trigbtnAddEnd, trigbtnCopy, trigbtnEdit,
		trigbtnDelete, trigbtnPasteNewCursor, trigbtnPasteNewEnd, trigbtnPaste, trigbtnUp, trigbtnDown;
	
	string h_initd[8], h_attribute[4], h_attribyte[8],
		h_attrishort[8], h_flag[16];
	string l_initd[8], l_attribute[4], l_attribyte[8],
		l_attrishort[8], l_flag[16];
	
	vector<string> warnings;
	
	int32_t index;
	int32_t trigger_index = 0;
	newcombo local_comboref;
	
	GUI::ListData list_ctype, list_flag, list_combscript, list_sfx, list_genscr,
		list_ss_counters_nn, list_sprites, list_sprites_spec, list_weaptype, list_deftypes,
		list_lift_parent_items, list_sprites_0none, list_dirs4n, list_0_7,
		list_light_shapes, list_triggers, list_strings, list_items, list_items_0none;
	
	
	std::shared_ptr<GUI::Widget> ANIM_FIELD_IMPL(byte* data, byte min, byte max);
	std::shared_ptr<GUI::Widget> CMB_INITD(int index);
	std::shared_ptr<GUI::Widget> CMB_FLAG(int index);
	std::shared_ptr<GUI::Widget> CMB_ATTRIBYTE(int index);
	std::shared_ptr<GUI::Widget> CMB_ATTRISHORT(int index);
	std::shared_ptr<GUI::Widget> CMB_ATTRIBUTE(int index);
	friend bool call_combo_editor(int32_t index);
	friend bool call_trigger_editor(ComboEditorDialog& dlg, size_t index);
	friend class ComboWizardDialog;
	friend class ComboTriggerDialog;
};

#endif
