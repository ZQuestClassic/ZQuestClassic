#ifndef ZC_DIALOG_COMBOEDITOR_H
#define ZC_DIALOG_COMBOEDITOR_H

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <gui/seltile_swatch.h>
#include <gui/selcombo_swatch.h>
#include <zq/gui/tileanim_frame.h>
#include <zq/gui/cornerselect.h>
#include <functional>
#include <string_view>
#include <map>

class ComboWizardDialog;
bool call_combo_editor(int32_t index);

std::string getComboTypeHelpText(int32_t id);
std::string getMapFlagHelpText(int32_t id);
int32_t solidity_to_flag(int32_t val);

class ComboEditorDialog: public GUI::Dialog<ComboEditorDialog>
{
public:
	enum class message { OK, CANCEL, COMBOTYPE, COMBOFLAG, HFLIP, VFLIP,
		ROTATE, PLUSCS, MINUSCS, CLEAR, DEFAULT, TILESEL, WIZARD };
	

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
	void flipSwatches(int rot, int hflip, int vflip);
	void updateFlip(int nflip);
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_initd[2];
	std::shared_ptr<GUI::Label> l_initds[2];
	std::shared_ptr<GUI::Label> l_attributes[4];
	std::shared_ptr<GUI::Label> l_attribytes[8];
	std::shared_ptr<GUI::Label> l_attrishorts[8];
	std::shared_ptr<GUI::Checkbox> l_flags[16];
	std::shared_ptr<GUI::Checkbox> cteff_tflag;
	std::shared_ptr<GUI::Button> ib_initds[2];
	std::shared_ptr<GUI::Button> ib_attributes[4];
	std::shared_ptr<GUI::Button> ib_attribytes[8];
	std::shared_ptr<GUI::Button> ib_attrishorts[8];
	std::shared_ptr<GUI::Button> ib_flags[16];
	std::shared_ptr<GUI::Button> wizardButton;
	std::shared_ptr<GUI::Label> l_flip;
	std::shared_ptr<GUI::Label> l_cset;
	std::shared_ptr<GUI::Label> l_minmax_trig;
	std::shared_ptr<GUI::CornerSwatch> cswatchs[3];
	std::shared_ptr<GUI::SelTileSwatch> tswatch;
	std::shared_ptr<GUI::SelComboSwatch> cycleswatch;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::string h_initd[2], h_attribute[4], h_attribyte[8],
		h_attrishort[8], h_flag[16];
	std::string l_initd[2], l_attribute[4], l_attribyte[8],
		l_attrishort[8], l_flag[16];
	int32_t index;
	newcombo local_comboref;
	GUI::ListData list_ctype, list_flag, list_combscript, list_sfx, list_genscr,
		list_counters_nn, list_sprites, list_sprites_spec, list_weaptype, list_deftypes;
	std::shared_ptr<GUI::Widget> ANIM_FIELD_IMPL(byte* data, byte min, byte max);
	std::shared_ptr<GUI::Widget> CMB_INITD(int index);
	std::shared_ptr<GUI::Widget> CMB_FLAG(int index);
	std::shared_ptr<GUI::Widget> CMB_ATTRIBYTE(int index);
	std::shared_ptr<GUI::Widget> CMB_ATTRISHORT(int index);
	std::shared_ptr<GUI::Widget> CMB_ATTRIBUTE(int index);
	std::shared_ptr<GUI::Checkbox> TRIGFLAG(int index, const char* str, int cspan = 1);
	friend bool call_combo_editor(int32_t index);
	friend class ComboWizardDialog;
};

#endif
