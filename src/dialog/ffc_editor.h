#ifndef ZC_DIALOG_FFCDLG_H_
#define ZC_DIALOG_FFCDLG_H_

#include "ffc.h"
#include <gui/dialog.h>
#include <gui/grid.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <zq/gui/selcombo_swatch.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <array>

struct ffdata;
bool call_ffc_dialog(int32_t ffcombo, mapscr* scr, int screen);
bool call_ffc_dialog(int32_t ffcombo, ffdata const& init, mapscr* scr, int screen);

struct ffdata
{
	int32_t x, y, dx, dy, ax, ay;
	word data;
	byte cset;
	word delay;
	ffc_flags flags;
	word link;
	int8_t layer = 1;
	byte twid : 2;
	byte fwid : 6 = 15;
	byte thei : 2;
	byte fhei : 6 = 15;
	script_config scrconfig;
	int32_t viewport_suspend_range;
	int32_t viewport_despawn_range;
	
	ffdata() = default;
	ffdata(mapscr* scr, int32_t ind);
	void clear();
	void load(mapscr* scr, int32_t ind);
	void save(int screen, int32_t ind);
	ffdata& operator=(ffdata const& other);
};

class FFCDialog: public GUI::Dialog<FFCDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, PLUSCS, MINUSCS };

	FFCDialog(mapscr* scr, int32_t screen, int32_t ffind);
	FFCDialog(mapscr* scr, int32_t screen, int32_t ffind, ffdata const& init);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	
	void post_realize() override;
private:
	void refreshSize();
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::SelComboSwatch> cmbsw;
	
	std::shared_ptr<GUI::Grid> cmb_container;
	
	ffdata ffc;
	int32_t screen;
	int32_t ffind;
	
	int prev_cache_x, prev_cache_y;
	GUI::ListData list_link, list_ffcscript;
};

#endif
