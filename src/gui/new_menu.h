#ifndef ZC_NEWMENU_H
#define ZC_NEWMENU_H

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

class MenuItem
{
public:
	MenuItem(string text = "", GuiMenu* submenu = nullptr, optional<uint> uid = nullopt, bool dis = false, bool sel = false);
	MenuItem(string text, string subtext, GuiMenu* submenu = nullptr, optional<uint> uid = nullopt, bool dis = false, bool sel = false);
	
	MenuItem(string text, std::function<void()> callback, optional<uint> uid = nullopt, bool dis = false, bool sel = false);
	MenuItem(string text, string subtext, std::function<void()> callback, optional<uint> uid = nullopt, bool dis = false, bool sel = false);
	
	MenuItem(MenuItem const& other) = default;
	MenuItem(MenuItem&& other) = default;
	MenuItem& operator=(MenuItem const& other) = default;
	MenuItem& operator=(MenuItem&& other) = default;
	
	void exec() const;
	void pop(uint x, uint y, GuiMenu* parent = nullptr);
	void draw(BITMAP* dest, uint x, uint y, uint style, byte drawflags, optional<uint> usew = nullopt);
	
	uint text_width() const;
	uint calc_width(uint style, uint drawflags) const;
	uint calc_height(uint style, uint drawflags) const;
	
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
	
	bool disabled;
	bool selected;
private:
	static constexpr uint vborder = 2, lborder = 4, rborder = 4;
	static constexpr uint t_vborder = 4, t_hborder = 8;
	static constexpr uint selindent = 12;
	static constexpr uint subindent = 8;
	friend class GuiMenu;
	friend class NewMenu;
	friend class TopMenu;
};
class GuiMenu
{
public:
	static void set_font(FONT* newfont);
	
	GuiMenu() = default;
	GuiMenu(std::initializer_list<MenuItem>&& entries);
	
	MenuItem* by_uid(uint uid);
	MenuItem* at(uint indx);
	MenuItem* hovered();
	
	virtual void reset_state();
	optional<uint> press_shortcut(char c);
	optional<uint> press_shortcut_key(char c);
	
	bool has_selected() const;
	bool has_doublewide() const;
	
	virtual optional<uint> hovered_ind() const = 0;
	virtual uint get_x(uint indx) const = 0;
	virtual uint get_y(uint indx) const = 0;
	virtual uint width() const = 0;
	virtual uint height() const = 0;
	
	virtual void pop(uint x, uint y, GuiMenu* parent = nullptr, FONT* f = nullptr) = 0;
	virtual void draw(BITMAP* dest, uint x, uint y, optional<uint> hl = nullopt) = 0;
	virtual void run_loop(uint x, uint y, GuiMenu* parent = nullptr) = 0;
	virtual bool run(uint x, uint y, GuiMenu* parent = nullptr) = 0;
	
	void disable_uid(uint uid, bool dis);
	void select_uid(uint uid, bool sel);
protected:
	vector<MenuItem> entries;
	optional<uint> sel_ind;
	optional<int> old_mb;
};

class NewMenu : public GuiMenu
{
public:
	NewMenu() = default;
	NewMenu(std::initializer_list<MenuItem>&& entries);
	
	optional<uint> hovered_ind() const override;
	
	uint get_x(uint indx) const override;
	uint get_y(uint indx) const override;
	uint width() const override;
	uint height() const override;
	
	void pop(uint x, uint y, GuiMenu* parent = nullptr, FONT* f = nullptr) override;
	void draw(BITMAP* dest, uint x, uint y, optional<uint> hl = nullopt) override;
	void run_loop(uint x, uint y, GuiMenu* parent = nullptr) override;
	bool run(uint x, uint y, GuiMenu* parent = nullptr) override;
private:
	static constexpr uint border = 2;
};
class TopMenu : public GuiMenu
{
public:
	TopMenu() = default;
	TopMenu(std::initializer_list<MenuItem>&& entries);
	
	optional<uint> hovered_ind() const override;
	
	uint get_x(uint indx) const override;
	uint get_y(uint indx) const override;
	uint width() const override;
	uint height() const override;
	
	void pop(uint x, uint y, GuiMenu* parent = nullptr, FONT* f = nullptr) override;
	void draw(BITMAP* dest, uint x, uint y, optional<uint> hl = nullopt) override;
	void run_loop(uint x, uint y, GuiMenu* parent = nullptr) override;
	bool run(uint x, uint y, GuiMenu* parent = nullptr) override;
private:
	static constexpr uint hborder = 0, vborder = 2;
};

#endif

