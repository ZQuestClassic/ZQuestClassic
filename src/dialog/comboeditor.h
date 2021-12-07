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
#include <gui/tileanim_frame.h>
#include <gui/cornerselect.h>
#include <functional>
#include <string_view>
#include <map>

bool call_combo_editor(int32_t index);

class ComboEditorDialog: public GUI::Dialog<ComboEditorDialog>
{
public:
	enum class message { OK, CANCEL, COMBOTYPE, COMBOFLAG, HFLIP, VFLIP, ROTATE, PLUSCS, MINUSCS, CLEAR };
	

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	ComboEditorDialog(newcombo const& ref, int32_t index, bool clrd = false);
	ComboEditorDialog(int32_t index, bool clrd = false);
	void loadComboType();
	void loadComboFlag();
	void updateCSet();
	void updateAnimation();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> l_attributes[4];
	std::shared_ptr<GUI::Label> l_attribytes[8];
	std::shared_ptr<GUI::Label> l_attrishorts[8];
	std::shared_ptr<GUI::Checkbox> l_flags[16];
	std::shared_ptr<GUI::Button> ib_attributes[4];
	std::shared_ptr<GUI::Button> ib_attribytes[8];
	std::shared_ptr<GUI::Button> ib_attrishorts[8];
	std::shared_ptr<GUI::Button> ib_flags[16];
	std::shared_ptr<GUI::Label> l_flip;
	std::shared_ptr<GUI::Label> l_cset;
	std::shared_ptr<GUI::CornerSwatch> cswatchs[3];
	std::shared_ptr<GUI::SelTileSwatch> tswatch;
	std::shared_ptr<GUI::SelComboSwatch> cycleswatch;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::string h_attribute[4], h_attribyte[8], h_attrishort[8], h_flag[16];
	int32_t index;
	newcombo local_comboref;
	std::string typehelp, flaghelp;
	int32_t lasttype;
	GUI::ListData list_ctype, list_flag, list_combscript,
		list_counters, list_sprites, list_weaptype, list_deftypes;
	friend bool call_combo_editor(int32_t index);
};

#endif
