#ifndef ZC_NEWMENU_H
#define ZC_NEWMENU_H

#include "base/headers.h"
#include "base/containers.h"
#include <functional>
#include "base/zc_alleg.h"


class NewMenu;

#define MENU_DRAWFLAG_HIGHLIGHT  0x01
#define MENU_DRAWFLAG_SHIFTSUBTX 0x02

class MenuItem
{
public:
	MenuItem(string text = "", NewMenu* submenu = nullptr, uint uid = 0, bool dis = false, bool sel = false);
	MenuItem(string text, std::function<void()> callback, uint uid = 0, bool dis = false, bool sel = false);
	MenuItem(string text, string subtext, NewMenu* submenu = nullptr, uint uid = 0, bool dis = false, bool sel = false);
	MenuItem(string text, string subtext, std::function<void()> callback, uint uid = 0, bool dis = false, bool sel = false);
	
	void exec() const;
	void pop(uint x, uint y);
	void draw(BITMAP* dest, uint x, uint y, uint w, byte drawflags);
	
	uint text_width() const;
	uint extra_width() const;
	uint width() const;
	uint height() const;
	
	bool isDisabled() const;
	bool isEmpty() const;
	bool isParent() const;
	bool hasDoubleExtra() const;
	
	void disable(bool dis);
	void select(bool sel);
protected:
	string text, subtext;
	uint uid;
	std::function<void()> callback;
	NewMenu* submenu;
	
	bool disabled;
	bool selected;
private:
	static constexpr uint vborder = 2, lborder = 16, rborder = 4;
	static constexpr uint subindent = 8;
	friend class NewMenu;
};
class NewMenu
{
public:
	NewMenu() = default;
	NewMenu(std::initializer_list<MenuItem>&& entries);
	
	MenuItem* by_uid(uint uid);
	MenuItem* at(uint indx);
	MenuItem* hovered();
	optional<uint> hovered_ind() const;
	
	uint get_x(uint indx) const;
	uint get_y(uint indx) const;
	
	uint width() const;
	uint height() const;
	
	void pop(uint x, uint y, FONT* f = nullptr);
	void draw(BITMAP* dest, uint x, uint y, optional<uint> hl = nullopt);
protected:
	vector<MenuItem> entries;
	
	void run(uint x, uint y);
private:
	static constexpr uint border = 2;
};

#endif

