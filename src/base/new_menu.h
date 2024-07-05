#ifndef ZC_NEWMENU_H_
#define ZC_NEWMENU_H_

#include "base/headers.h"
#include "base/containers.h"
#include <functional>
#include "base/zc_alleg.h"


class GuiMenu;

#define MENU_DRAWFLAG_HIGHLIGHT  0x01
#define MENU_DRAWFLAG_SHIFTSUBTX 0x02
#define MENU_DRAWFLAG_INVFRAME   0x04
#define MENU_DRAWFLAG_NOSEL      0x08

enum
{
	MISTYLE_POPUP,
	MISTYLE_TOP,
	MISTYLE_MAX
};

enum MenuRet
{
	MRET_NIL,
	MRET_LEFT,
	MRET_RIGHT,
};
#define MFL_DIS            0x00000001
#define MFL_SEL            0x00000002
#define MFL_EXIT_PRE_PROC  0x00000004
class MenuItem
{
public:
	MenuItem(string text = "", GuiMenu* submenu = nullptr, optional<uint> uid = nullopt, int32_t flags = 0);
	MenuItem(string text, string subtext, GuiMenu* submenu = nullptr, optional<uint> uid = nullopt, int32_t flags = 0);
	
	MenuItem(string text, std::function<void()> callback, optional<uint> uid = nullopt, int32_t flags = 0);
	MenuItem(string text, string subtext, std::function<void()> callback, optional<uint> uid = nullopt, int32_t flags = 0);
	
	MenuItem(optional<uint> uid);
	
	MenuItem(MenuItem const& other) = default;
	MenuItem(MenuItem&& other) = default;
	MenuItem& operator=(MenuItem const& other) = default;
	MenuItem& operator=(MenuItem&& other) = default;
	
	void exec() const;
	MenuRet pop(uint x, uint y, GuiMenu* parent = nullptr);
	void draw(BITMAP* dest, uint x, uint y, uint style, byte drawflags, optional<FONT*> usefont, optional<uint> usew = nullopt);
	
	uint calc_width(uint style, uint drawflags, optional<FONT*> usefont) const;
	uint calc_height(uint style, uint drawflags, optional<FONT*> usefont) const;
	
	bool isDisabled() const;
	bool isSelected() const;
	bool isEmpty() const;
	bool isParent() const;
	
	void disable(bool dis);
	void select(bool sel);
	
	string text, subtext;
	optional<uint> uid;
	std::function<void()> onCall;
	GuiMenu* submenu;
	
private:
	int32_t flags;
	static constexpr uint vborder = 4, lborder = 6, rborder = 4;
	static constexpr uint t_vborder = 4, t_hborder = 8;
	static constexpr uint selindent = 12, sel_antiindent = 4;
	static constexpr uint subindent = 8;
	static constexpr uint hl_border = 1;
	static constexpr uint empty_height = 6;
	friend class GuiMenu;
	friend class NewMenu;
	friend class TopMenu;
};

struct MenuState
{
	optional<uint> sel_ind;
	optional<int> old_mb;
	bool dirty = true;
	void clear();
};

class GuiMenu
{
public:
	GuiMenu() = default;
	GuiMenu(std::initializer_list<MenuItem>&& entries);
	GuiMenu(optional<uint> chop, bool borderless);
	GuiMenu(optional<uint> chop, bool borderless,std::initializer_list<MenuItem>&& entries);
	
	static int proc(int msg, DIALOG* d, int c);
	void do_draw();
	
	virtual void reset_state();
	virtual optional<uint> hovered_ind() const = 0;
	virtual bool has_mouse() const = 0;
	virtual optional<uint> get_x(uint indx) const = 0;
	virtual optional<uint> get_y(uint indx) const = 0;
	virtual uint width() const = 0;
	virtual uint height() const = 0;
	
	virtual MenuRet pop(uint x, uint y, GuiMenu* parent = nullptr) = 0;
	virtual void pop_sub(uint indx, GuiMenu* parent = nullptr) = 0;
	virtual void draw(BITMAP* dest, optional<uint> hl = nullopt) = 0;
	virtual MenuRet run_loop(GuiMenu* parent = nullptr) = 0;
	virtual void run(bool allow_focus = false, GuiMenu* parent = nullptr) = 0;
	
	virtual void trigger(uint indx) = 0;
	
	void add(MenuItem const& entry);
	void add(MenuItem&& entry);
	void add(std::initializer_list<MenuItem> const& entries);
	void add_sep();
	
	optional<uint> ind_at(uint uid);
	MenuItem* by_uid(uint uid);
	MenuItem* at(uint indx);
	MenuItem* hovered();
	uint chop_sz() const;
	size_t size() const {return entries.size();}
	vector<MenuItem>& inner() {return entries;}
	vector<MenuItem> const& inner() const {return entries;}
	
	void setFont(optional<FONT*> usefont) {menu_font = usefont;}
	
	optional<uint> press_shortcut(char c);
	optional<uint> press_shortcut_key(char c);
	
	bool has_selected() const;
	bool has_doublewide() const;
	void select_only_index(uint indx);
	void select_only_uid(uint uid);
	void for_each(std::function<void(MenuItem&,uint)> proc);
	void disable_uid(uint uid, bool dis);
	void select_uid(uint uid, bool sel);
	void position(uint x, uint y);
	
	optional<uint> chop_index;
	bool borderless;
protected:
	MenuState state;
	vector<MenuItem> entries;
	optional<FONT*> menu_font;
	uint xpos, ypos;
};
class NewMenu : public GuiMenu
{
public:
	NewMenu() = default;
	NewMenu(std::initializer_list<MenuItem>&& entries);
	NewMenu(optional<uint> chop, bool borderless);
	NewMenu(optional<uint> chop, bool borderless,std::initializer_list<MenuItem>&& entries);
	
	optional<uint> hovered_ind() const override;
	bool has_mouse() const override;
	
	optional<uint> get_x(uint indx) const override;
	optional<uint> get_y(uint indx) const override;
	uint width() const override;
	uint height() const override;
	
	void trigger(uint indx) override;
	
	MenuRet pop(uint x, uint y, GuiMenu* parent = nullptr) override;
	void pop_sub(uint indx, GuiMenu* parent = nullptr) override;
	void draw(BITMAP* dest, optional<uint> hl = nullopt) override;
	MenuRet run_loop(GuiMenu* parent = nullptr) override;
	void run(bool allow_focus = false, GuiMenu* parent = nullptr) override;
private:
	static constexpr uint border = 2;
};
class TopMenu : public GuiMenu
{
public:
	TopMenu() = default;
	TopMenu(std::initializer_list<MenuItem>&& entries);
	TopMenu(optional<uint> chop, bool borderless);
	TopMenu(optional<uint> chop, bool borderless,std::initializer_list<MenuItem>&& entries);
	
	optional<uint> hovered_ind() const override;
	bool has_mouse() const override;
	
	optional<uint> get_x(uint indx) const override;
	optional<uint> get_y(uint indx) const override;
	uint width() const override;
	uint height() const override;
	
	void trigger(uint indx) override;
	
	MenuRet pop(uint x, uint y, GuiMenu* parent = nullptr) override;
	void pop_sub(uint indx, GuiMenu* parent = nullptr) override;
	void draw(BITMAP* dest, optional<uint> hl = nullopt) override;
	MenuRet run_loop(GuiMenu* parent = nullptr) override;
	void run(bool allow_focus = false, GuiMenu* parent = nullptr) override;
private:
	static constexpr uint hborder = 0, vborder = 2;
};

optional<int> popup_num_menu(uint x, uint y, int min, int max, optional<int> hl = nullopt, std::function<string(int)> formatter = nullptr);

#endif

