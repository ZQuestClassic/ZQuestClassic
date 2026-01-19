#ifndef ZC_DIALOG_INFOLISTER_H_
#define ZC_DIALOG_INFOLISTER_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/list.h>
#include <gui/grid.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/window.h>
#include <zq/gui/tileanim_frame.h>
#include <zq/gui/dmap_frame.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>

extern int lister_sel_val;

class BasicListerDialog: public GUI::Dialog<BasicListerDialog>
{
public:
	enum class message { REFR_INFO, OK, EDIT, EXIT, COPY, PASTE, ADV_PASTE, SAVE, LOAD, CONFIRM, CLEAR };
	
	BasicListerDialog(std::string title, std::string cfg_key, int start_val = 0, bool selecting = false) :
		titleTxt(title), selected_val(start_val), start_val(start_val), frozen_start(0), frozen_end(0),
		selecting(selecting), use_preview(false), editable(true), alphabetized(false),
		use_mappreview(false), use_alpha(true), cfg_key(cfg_key){};
	BasicListerDialog(std::string title, std::string cfg_key, GUI::ListData lister, int start_val = 0, bool selecting = false) :
		titleTxt(title), lister(lister), selected_val(start_val), start_val(start_val), frozen_start(0), frozen_end(0),
		selecting(selecting), use_preview(false), editable(true), alphabetized(false),
		use_mappreview(false), use_alpha(true), cfg_key(cfg_key){};
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
	
protected:
	virtual void preinit(){};
	virtual void postinit(){};
	virtual void update(bool startup = false){};
	virtual void edit(){};
	virtual void rclick(int x, int y){};
	virtual bool clear(){return false;};
	virtual void copy(){};
	virtual bool paste(){return false;};
	virtual bool adv_paste(){return false;};
	virtual void save(){};
	virtual bool load(){return false;};
	
	void resort();
	bool get_config(std::string const& name, bool default_val);
	int32_t get_config(std::string const& name, int32_t default_val);
	std::string get_config(std::string const& name, std::string const& default_val);
	double get_config(std::string const& name, double default_val);
	void set_config(std::string const& name, bool value);
	void set_config(std::string const& name, int32_t value);
	void set_config(std::string const& name, std::string const& value);
	void set_config(std::string const& name, double value);
	
	std::string titleTxt;
	GUI::ListData lister;
	int selected_val, start_val;
	size_t frozen_start, frozen_end;
	bool selecting, use_preview, editable, alphabetized, use_mappreview, use_alpha;
	
	std::string cfg_key;
	
	std::shared_ptr<GUI::List> widgList;
	std::shared_ptr<GUI::Label> widgInfo;
	std::shared_ptr<GUI::TileFrame> widgPrev;
	std::shared_ptr<GUI::DMapFrame> mapPrev;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Grid> prev_holder;
	std::shared_ptr<GUI::Grid> btnrow, btnrow2;
};

class ItemListerDialog: public BasicListerDialog
{
public:
	ItemListerDialog(int itemid = -1, bool selecting = false);
	
protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	void copy() override;
	bool paste() override;
	bool adv_paste() override;
	void save() override;
	bool load() override;
};

class SpriteListerDialog: public BasicListerDialog
{
public:
	SpriteListerDialog(int spriteid = -1, bool selecting = false);
	
protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	void copy() override;
	bool paste() override;
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
	void update(bool startup = false) override;
};

class EnemyListerDialog : public BasicListerDialog
{
public:
	EnemyListerDialog(int enemyid = -1, bool selecting = false);

protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
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
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	bool clear_nondelete();
	bool clear() override;
	void copy() override;
	bool paste() override;

private:
	std::shared_ptr<GUI::Button> up_btn, down_btn, del_btn;
};

class MidiListerDialog : public BasicListerDialog
{
public:
	MidiListerDialog(int index = -1, bool selecting = false);

protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	bool clear() override;
	void copy() override;
	bool paste() override;
};

class DMapListerDialog : public BasicListerDialog
{
public:
	DMapListerDialog(int index = -1, bool selecting = false);

protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void copy() override;
	bool paste() override;
};

class FFCListerDialog : public BasicListerDialog
{
public:
	FFCListerDialog(int index = -1, bool selecting = false);

protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	bool clear() override;
	void copy() override;
	bool paste() override;
private:
	int cache_tw, cache_th;
	int cache_max_tw, cache_max_th;
};

class SaveMenuListerDialog : public BasicListerDialog
{
public:
	SaveMenuListerDialog(int index = -1, bool selecting = false);
	
protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	bool clear() override;
	void copy() override;
	bool paste() override;
};

class MusicListerDialog : public BasicListerDialog
{
public:
	MusicListerDialog(int index = -1, bool selecting = false);
	
protected:
	void preinit() override;
	void postinit() override;
	void update(bool startup = false) override;
	void edit() override;
	void rclick(int x, int y) override;
	bool clear_nondelete();
	bool clear() override;
	void copy() override;
	bool paste() override;

private:
	std::shared_ptr<GUI::Button> up_btn, down_btn, del_btn;
};
#endif
