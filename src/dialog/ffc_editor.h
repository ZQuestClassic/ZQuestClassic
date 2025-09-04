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
	int8_t layer;
	byte twid : 2;
	byte fwid : 6;
	byte thei : 2;
	byte fhei : 6;
	int32_t script;
	std::array<int, 8> initd;
	
	ffdata();
	ffdata(mapscr* scr, int32_t ind);
	void clear();
	void load(mapscr* scr, int32_t ind);
	void save(mapscr* scr, int screen, int32_t ind);
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
	std::shared_ptr<GUI::Widget> FFC_INITD(int index);
	void refreshScript();
	void refreshSize();
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::SelComboSwatch> cmbsw;
	
	std::string h_initd[8];
	std::shared_ptr<GUI::TextField> tf_initd[8];
	std::shared_ptr<GUI::Button> ib_initds[8];
	std::shared_ptr<GUI::Label> l_initds[8];
	std::shared_ptr<GUI::Grid> cmb_container;
	
	ffdata ffc;
	mapscr* thescr;
	int32_t screen;
	int32_t ffind;
	GUI::ListData list_link, list_ffcscript;
};

#endif
