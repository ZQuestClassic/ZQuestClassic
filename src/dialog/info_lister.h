#ifndef ZC_DIALOG_INFOLISTER_H_
#define ZC_DIALOG_INFOLISTER_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/list.h>
#include <gui/window.h>
#include <zq/gui/tileanim_frame.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>

extern int lister_sel_val;

class BasicListerDialog: public GUI::Dialog<BasicListerDialog>
{
public:
	enum class message { REFR_INFO, OK, EDIT, EXIT, COPY, PASTE, ADV_PASTE, SAVE, LOAD, CONFIRM };
	
	BasicListerDialog(std::string title, int start_val = 0, bool selecting = false) :
		titleTxt(title), selected_val(start_val), start_val(start_val), selecting(selecting),
		use_preview(false), editable(true){};
	BasicListerDialog(std::string title, GUI::ListData lister, int start_val = 0, bool selecting = false) :
		titleTxt(title), lister(lister), selected_val(start_val), start_val(start_val),
		selecting(selecting), use_preview(false), editable(true){};
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	virtual void preinit(){};
	virtual void postinit(){};
	virtual void update(){};
	virtual void edit(){};
	virtual void rclick(int x, int y){};
	virtual void copy(){};
	virtual bool paste(){return false;};
	virtual bool adv_paste(){return false;};
	virtual void save(){};
	virtual bool load(){return false;};
	
	std::string titleTxt;
	GUI::ListData lister;
	int selected_val, start_val;
	bool selecting, use_preview, editable;
	
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
	void preinit() override;
	void postinit() override;
	void update() override;
	void edit() override;
	void rclick(int x, int y) override;
	void copy() override;
	bool paste() override;
	bool adv_paste() override;
	void save() override;
	bool load() override;
};

class SubscrWidgListerDialog: public BasicListerDialog
{
public:
	SubscrWidgListerDialog();
	
protected:
	void preinit() override;
	void postinit() override;
	void update() override;
};

class EnemyListerDialog : public BasicListerDialog
{
public:
	EnemyListerDialog(int enemyid = -1, bool selecting = false);

protected:
	void preinit() override;
	void postinit() override;
	void update() override;
	void edit() override;
	void rclick(int x, int y) override;
	void copy() override;
	bool paste() override;
	void save() override;
	bool load() override;
};

class SFXListerDialog : public BasicListerDialog
{
public:
	SFXListerDialog(int sfxid = -1, bool selecting = false);

protected:
	void edit() override;
};

class MidiListerDialog : public BasicListerDialog
{
public:
	MidiListerDialog(int index = -1, bool selecting = false);

protected:
	void preinit() override;
	void postinit() override;
	void update() override;
	void edit() override;
};
#endif
