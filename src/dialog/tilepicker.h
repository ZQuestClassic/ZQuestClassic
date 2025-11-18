#ifndef ZC_DIALOG_TILESPICKER_H_
#define ZC_DIALOG_TILESPICKER_H_

#include "base/headers.h"
#include "base/containers.h"
#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/grid.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>
#include "dialog/externs.h"

struct TilePickerData
{
	int tile;
	byte cset;
	byte flip;
};

bool call_tilepicker_dlg(string const& title, vector<vector<TilePickerData>>& data);

class TilePickerDialog: public GUI::Dialog<TilePickerDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	TilePickerDialog(string const& title, bool& confirm, vector<vector<TilePickerData>>& data);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	string d_title;
	vector<vector<TilePickerData>>& data;
	bool& confirm;
	std::shared_ptr<GUI::Window> window;
};

#endif
