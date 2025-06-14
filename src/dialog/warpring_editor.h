#ifndef ZC_DIALOG_WARPRING_EDITOR_H_
#define ZC_DIALOG_WARPRING_EDITOR_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list.h>
#include <gui/list_data.h>
#include <gui/drop_down_list.h>
#include <zq/gui/dmap_mapgrid.h>
#include <zq/gui/dmap_frame.h>
#include <zq/gui/tileanim_frame.h>
#include <functional>
#include <string_view>

//all of this uniquely is for editing warprings, so it is all in two files
//warpring_editor.h and warpring_editor.cpp 
//I've sectioned off everything with comments.

//Call functions to open the dialogs, these are called in reverse order
void call_warpring_editor(int32_t ring, byte index);

void call_warpring_warpselector(int32_t ring);

void call_warpring_ringselector();

//Warpring Warp Editor Dialog
class Warpring_Editor_Dialog : public GUI::Dialog<Warpring_Editor_Dialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL};


	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	Warpring_Editor_Dialog(int32_t ring, byte index);

	byte ring=0;
	byte index=0;

	GUI::ListData list_dmaps;

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::DropDownList> w_dmaplist;
	std::shared_ptr<GUI::TextField> w_screen;
	std::shared_ptr<GUI::DMapMapGrid> w_dmapgrid;
	std::shared_ptr<GUI::DMapFrame> w_dmapframe;

	friend void call_warpring_editor(int32_t ring, byte index);
};

//Warp Selector Dialog

class Warpring_WarpSelector_Dialog : public GUI::Dialog<Warpring_WarpSelector_Dialog>
{
public:
	enum class message { REFR_INFO, EDIT, DONE, CHANGEWARP};


	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	Warpring_WarpSelector_Dialog(int32_t ring);

	byte ring = 0;
	byte index = 0;
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::List> w_warplist;
	std::shared_ptr<GUI::TextField> w_size;
	std::shared_ptr<GUI::DMapFrame> w_dmapframe;
	std::shared_ptr<GUI::Label> l_map, l_screen;

	GUI::ListData lister;

	friend void call_warpring_warpselector(int32_t ring);
};

//Warpring Ring Selector Dialog

class Warpring_RingSelector_Dialog : public GUI::Dialog<Warpring_RingSelector_Dialog>
{
public:
	enum class message { REFR_INFO, EDIT, CANCEL, CHANGERING};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	Warpring_RingSelector_Dialog();

	byte ring;

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::List> w_ringlist;

	GUI::ListData lister;

	friend void call_warpring_ringselector();
};

#endif