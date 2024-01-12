#ifndef ZC_DIALOG_SCREEN_NOTES_H
#define ZC_DIALOG_SCREEN_NOTES_H

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/list.h>
#include <gui/window.h>
#include <zq/gui/tileanim_frame.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>

struct ScreenNotesInfo
{
	ScreenNotesInfo(mapscr* scr, int map, int screen)
		: scr(scr), map(map), screen(screen),
		blank(false)
	{
		update_blank();
	}
	mapscr* scr;
	int map, screen;
	
	bool isBlank() const {return blank;}
	string const& notes() const;
	string title() const;
	string str() const;
	bool edit();
private:
	void update_blank();
	bool blank;
};

class BrowseNotesDialog: public GUI::Dialog<BrowseNotesDialog>
{
public:
	enum class message { REFR_INFO, OK, EDIT };
	
	BrowseNotesDialog();
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	GUI::ListData lister;
	int selected_val;
	vector<ScreenNotesInfo> infos;
	string filter;
	ScreenNotesInfo* sel_info();
	ScreenNotesInfo const* sel_info() const;
	
	std::shared_ptr<GUI::List> widgList;
	std::shared_ptr<GUI::Label> prevlbl;
	std::shared_ptr<GUI::Window> window;
};

#endif
