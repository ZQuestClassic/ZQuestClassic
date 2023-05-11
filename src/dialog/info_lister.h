#ifndef ZC_DIALOG_INFOLISTER_H
#define ZC_DIALOG_INFOLISTER_H

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/list.h>
#include <gui/window.h>
#include <zq/gui/tileanim_frame.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>

extern int lister_index;

class BasicListerDialog: public GUI::Dialog<BasicListerDialog>
{
public:
	enum class message { OK, EDIT, EXIT, COPY, PASTE, SAVE, LOAD };
	
	BasicListerDialog(std::string title, int start_ind = 0, bool selecting = false) :
		titleTxt(title), start_ind(start_ind), selected_index(start_ind), selecting(selecting){};
	BasicListerDialog(std::string title, GUI::ListData lister, int start_ind = 0, bool selecting = false) :
		titleTxt(title), start_ind(start_ind), selected_index(start_ind), selecting(selecting), lister(lister){};
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	virtual void postinit(){};
	virtual void update(){};
	virtual void edit(){};
	virtual void rclick(int x, int y){};
	virtual void copy(){};
	virtual bool paste(){return false;};
	virtual void save(){};
	virtual bool load(){return false;};
	
	std::string titleTxt;
	GUI::ListData lister;
	int selected_index, start_ind;
	bool selecting;
	
	std::shared_ptr<GUI::List> widgList;
	std::shared_ptr<GUI::Label> widgInfo;
	std::shared_ptr<GUI::TileFrame> widgPrev;
	std::shared_ptr<GUI::Window> window;
};

class ItemListerDialog: public BasicListerDialog
{
public:
	ItemListerDialog(int itemid = -1, bool selecting = false);
	
protected:
	void postinit() override;
	void update() override;
	void edit() override;
	void rclick(int x, int y) override;
	void copy() override;
	bool paste() override;
	void save() override;
	bool load() override;
};

#endif
