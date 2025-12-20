#include "gui/button.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "gui/jwin_a5.h"
#include "base/general.h"
#include <algorithm>
#include <utility>

void zq_push_unfrozen_dialogs(size_t value);
void zq_pop_unfrozen_dialogs();

bool is_reserved_key(int c);
bool is_reserved_keycombo(int c, int modflag);

static std::set<int> held_mod_keys;
static const int mod_keys[] =
{
	KEY_LSHIFT, KEY_RSHIFT,
	KEY_LCONTROL, KEY_RCONTROL,
	KEY_COMMAND,
	KEY_ALT, KEY_ALTGR,
};
static const string hotkey_index_names[] =
{
	"Main", "Alternate"
};
static bool waiting_no_buttons = false;
static void reset_held_mod_keys()
{
	poll_keyboard();
	for (auto k : mod_keys)
	{
		if (key[k])
			held_mod_keys.insert(k);
		else held_mod_keys.erase(k);
	}
}

static ALLEGRO_JOYSTICK* binding_joystick;

void set_binding_joystick(ALLEGRO_JOYSTICK* stick)
{
	binding_joystick = stick;
}

bool joybtn(int stick_idx, int b)
{
	if(b == 0)
		return false;
	if (b-1 >= joy[stick_idx].num_buttons)
		return false;
		
	return joy[stick_idx].button[b-1].b !=0;
}
bool joystick(int stick_idx, int s)
{
	if(s < 0)
		return false;
	if (s >= joy[stick_idx].num_sticks)
		return false;
	
	for (int i = 0; i < joy[stick_idx].stick[s].num_axis; i++)
	{
		if (joy[stick_idx].stick[s].axis[i].d1 || joy[stick_idx].stick[s].axis[i].d2)
			return true;
	}
	return false;
}
static optional<int> get_btnpress(int stick_idx, bool stick)
{
	if (stick)
	{
		if (!binding_joystick || !al_get_joystick_active(binding_joystick))
			return -1;
		for(int q = 0; q < joy[stick_idx].num_sticks; ++q)
			if(joystick (stick_idx, q))
				return q;
	}
	else
	{
		for(int q = 1; q <= joy[stick_idx].num_buttons; ++q)
			if(joybtn (stick_idx, q))
				return q;
	}
	return std::nullopt;
}
optional<int> get_next_btnpress(int stick_idx, bool stick)
{
	poll_joystick();
	auto ret = get_btnpress(stick_idx, stick);
	if (ret && *ret < 0) return ret;
	if (waiting_no_buttons)
	{
		if (!ret)
			waiting_no_buttons = false;
		return std::nullopt;
	}
	return ret;
}
optional<int> get_next_keypress(bool check_mod_keys)
{
	if (keypressed())
	{
		int val, scancode;
		val = ureadkey(&scancode);
		
		return scancode;
	}
	if (check_mod_keys)
	{
		for (auto k : mod_keys)
		{
			if (held_mod_keys.contains(k))
			{
				if (!key[k])
					held_mod_keys.erase(k);
			}
			else if (key[k])
			{
				held_mod_keys.insert(k);
				return k;
			}
		}
		poll_keyboard();
	}
	return std::nullopt;
}

void spinner_loop(vector<string> const& strs, std::function<bool()> proc)
{
	auto mz = mouse_z;
	
	FONT* popup_font = get_custom_font(CFONT_DLG);
	int tw = 0;
	for (string const& s : strs)
	{
		int w = text_length(popup_font, s.c_str());
		if (w > tw) tw = w;
	}
	const int fh = text_height(popup_font);
	const int vspacing = 3;
	const int th = (fh + vspacing) * strs.size() - vspacing;
	const int ar = th / 4; // arc radius for spinner
	const int hspacing = 5 + fh;
	const int hmargin = 32;
	const int vmargin = 16;
	const int popup_w = hmargin * 2 + tw + hspacing + ar * 2, popup_h = vmargin * 2 + th;
	const int popup_x = (screen->w - popup_w) / 2, popup_y = (screen->h - popup_h) / 2;
	const double width = (2*PI) / 3.0; // arc length for spinner, in radians
	const double thickness = 2.0; // thicknes of spinner, in pixels
	const double aspd = (2*PI) / 60.0; // arc speed of spinner, in radians/frame
	const int tx = popup_x + hmargin + tw / 2;
	const int ax = popup_x + hmargin + tw + hspacing + ar;
	const int ay = popup_y + popup_h / 2;
	
	
	popup_zqdialog_start();
	jwin_draw_win(screen, (screen->w-popup_w)/2, (screen->h-popup_h)/2, popup_w, popup_h, FR_WIN);
	
	int ty = popup_y + vmargin;
	for (string const& s : strs)
	{
		textout_centre_ex(screen, popup_font, s.c_str(), tx, ty, jwin_pal[jcBOXFG], jwin_pal[jcBOX]);
		ty += fh + vspacing;
	}
	clear_keybuf();
	popup_zqdialog_start_a5();
	zqdialog_set_skiptint(true);
	zq_push_unfrozen_dialogs(1);
	
	double a = 0.0;
	do
	{
		al_draw_arc(ax, ay, ar, a, width, a5color(jwin_pal[jcBOX]), thickness); //erase the last frame's arc
		a = wrap_float(a + aspd, 0.0, 2 * PI);
		al_draw_arc(ax, ay, ar, a, width, a5color(jwin_pal[jcBOXFG]), thickness); // draw the new frame's arc
		update_hw_screen();
	}
	while (!proc());
	
	while(gui_mouse_b())
		rest(1);
	clear_keybuf();
	
	zq_pop_unfrozen_dialogs();
	popup_zqdialog_end_a5();
	popup_zqdialog_end();
	
	position_mouse_z(mz);
}
void joy_getbtn(string const& title, int& btn_ref, int stick_idx, bool stick)
{
	vector<string> strs;
	if (stick)
		strs.emplace_back("Move a stick (or DPAD)");
	else strs.emplace_back("Press a button");
	if (!title.empty())
		strs.emplace_back(title);
	strs.emplace_back("ESC to cancel");
	strs.emplace_back("SPACE to clear");
	waiting_no_buttons = true;
	spinner_loop(strs, [&]()
		{
			while (auto key = get_next_keypress(false))
			{
				if (*key == KEY_ESC) // exit
					return true;
				if (*key == KEY_SPACE) // clear and exit
				{
					btn_ref = 0;
					return true;
				}
			}
			if (auto btn = get_next_btnpress(stick_idx, stick))
			{
				if (*btn >= 0) // negative indicates gamepad disconnection
					btn_ref = *btn;
				return true;
			}
			return false;
		});
}
void kb_getkey(string const& title, int& key_ref)
{
	vector<string> strs;
	strs.emplace_back("Press any key");
	if (!title.empty())
		strs.emplace_back(title);
	strs.emplace_back("ESC to cancel");
	reset_held_mod_keys();
	spinner_loop(strs, [&]()
		{
			while (auto key = get_next_keypress(true))
			{
				if (*key == KEY_ESC)
					return true; // exit
				if (*key >= KEY_F1 && *key <= KEY_F12 && *key != KEY_F11)
					continue; // disallow
				if (*key < 0 || *key > 123)
					continue; // out of range, disallow
				key_ref = *key;
				return true;
			}
			return false;
		});
}
void kb_clearkey(string const& title, int& key_ref)
{
	vector<string> strs;
	strs.emplace_back("Press any key");
	if (!title.empty())
		strs.emplace_back(title);
	strs.emplace_back("ESC to cancel");
	reset_held_mod_keys();
	spinner_loop(strs, [&]()
		{
			if (auto key = get_next_keypress(true))
			{
				if (*key == KEY_ESC)
					return true; // exit
				key_ref = 0;
				return true;
			}
			return false;
		});
}
void kb_get_hotkey(string const& title, int& hkey, int& modflag)
{
	vector<string> strs;
	strs.emplace_back("Press any key (+mods)");
	if (!title.empty())
		strs.emplace_back(title);
	strs.emplace_back("ESC to cancel");
	spinner_loop(strs, [&]()
		{
			while (auto key = get_next_keypress(false))
			{
				if (*key == KEY_ESC)
					return true; // exit
				if (*key < 0 || *key > 123)
					continue; // out of range, disallow
				if (is_modkey(*key))
					continue; // ignore
				int mods = get_mods();
				if (is_reserved_key(*key) || is_reserved_keycombo(*key, mods))
					continue; // disallow
				hkey = *key;
				modflag = mods;
				return true;
			}
			return false;
		});
}
void kb_clear_hotkey(string const& title, int& hkey, int& modflag)
{
	vector<string> strs;
	strs.emplace_back("Press any key");
	if (!title.empty())
		strs.emplace_back(title);
	strs.emplace_back("ESC to cancel");
	reset_held_mod_keys();
	spinner_loop(strs, [&]()
		{
			while (auto key = get_next_keypress(true))
			{
				if (*key == KEY_ESC)
					return true; // exit
				hkey = 0;
				modflag = 0;
				return true;
			}
			return false;
		});
}

namespace GUI
{

int d_kbutton_proc(int msg,DIALOG *d,int c)
{
	GUI::Button* b = (GUI::Button*)d->dp3;
	bool should_be_disabled = false;
	if(!b)
		should_be_disabled = true;
	else switch (b->btnType)
	{
		case GUI::Button::type::BIND_HOTKEY:
			if (!b->bound_hotkey || b->hotkeyindx > 1)
				should_be_disabled = true;
			break;
		case GUI::Button::type::BIND_HOTKEY_CLEAR:
			if (!b->bound_hotkey || b->hotkeyindx > 1)
				should_be_disabled = true;
			else if(!(b->bound_hotkey->hotkey[b->hotkeyindx] || b->bound_hotkey->modflag[b->hotkeyindx]))
				should_be_disabled = true; // nothing to clear!
			break;
		case GUI::Button::type::BIND_KB:
			if (!b->bound_key)
				should_be_disabled = true;
			break;
		case GUI::Button::type::BIND_KB_CLEAR:
			if (!b->bound_key)
				should_be_disabled = true;
			else if (!*(b->bound_key))
				should_be_disabled = true; // nothing to clear!
			break;
	}
	switch(msg)
	{
		case MSG_KEY:
		case MSG_CLICK:
		{
			if (should_be_disabled) break;
			
			d->flags |= D_SELECTED;
			jwin_button_proc(MSG_DRAW,d,0);
			switch (b->btnType)
			{
				case GUI::Button::type::BIND_KB:
					kb_getkey(fmt::format("for keybind '{}'", b->bind_name), *b->bound_key);
					break;
				case GUI::Button::type::BIND_HOTKEY:
				{
					Hotkey* hk = b->bound_hotkey;
					kb_get_hotkey(fmt::format("for '{}' - {} Key", b->bind_name, hotkey_index_names[b->hotkeyindx]),
						hk->hotkey[b->hotkeyindx], hk->modflag[b->hotkeyindx]);
					break;
				}
				
				case GUI::Button::type::BIND_KB_CLEAR:
					kb_clearkey(fmt::format("to clear keybind '{}'", b->bind_name), *b->bound_key);
					break;
				case GUI::Button::type::BIND_HOTKEY_CLEAR:
				{
					Hotkey* hk = b->bound_hotkey;
					kb_clear_hotkey(fmt::format("to clear '{}' - {} Key", b->bind_name, hotkey_index_names[b->hotkeyindx]),
						hk->hotkey[b->hotkeyindx], hk->modflag[b->hotkeyindx]);
					break;
				}
			}
			
			d->flags &= ~D_SELECTED;
			
			GUI_EVENT(d, geCLICK);
			return D_REDRAW;
		}
	}

	int f = d->flags;
	if(should_be_disabled)
		d->flags |= D_DISABLED;
	int ret = jwin_button_proc(msg,d,c);
	d->flags = f;
	return ret;
}

int d_joybutton_proc(int msg,DIALOG *d,int c)
{
	GUI::Button* b = (GUI::Button*)d->dp3;
	bool should_be_disabled = false;
	if(!b)
		should_be_disabled = true;
	else switch (b->btnType)
	{
		case GUI::Button::type::BIND_JOYKEY:
		case GUI::Button::type::BIND_JOYSTICK:
			if (!b->bound_key || !b->bound_stick_idx)
				should_be_disabled = true;
			break;
	}
	switch(msg)
	{
		case MSG_KEY:
		case MSG_CLICK:
		{
			if (should_be_disabled) break;
			
			d->flags |= D_SELECTED;
			jwin_button_proc(MSG_DRAW,d,0);
			
			switch (b->btnType)
			{
				case GUI::Button::type::BIND_JOYKEY:
					joy_getbtn(fmt::format("for button bind '{}'", b->bind_name), *b->bound_key, *b->bound_stick_idx, false);
					break;
				case GUI::Button::type::BIND_JOYSTICK:
					joy_getbtn(fmt::format("for stick bind '{}'", b->bind_name), *b->bound_key, *b->bound_stick_idx, true);
					break;
			}
			
			d->flags &= ~D_SELECTED;
			
			GUI_EVENT(d, geCLICK);
			return D_REDRAW;
		}
	}

	int f = d->flags;
	if(should_be_disabled)
		d->flags |= D_DISABLED;
	int ret = jwin_button_proc(msg,d,c);
	d->flags = f;
	return ret;
}

Button::Button(): text(), message(-1), btnType(type::BASIC), bound_key(nullptr),
	bound_hotkey(nullptr), hotkeyindx(0), icontype(BTNICON_ARROW_UP)
{
	setPreferredHeight(3_em);
}

void Button::setType(type newType)
{
	if(!alDialog)
		btnType = newType;
}
void Button::setBoundKB(int* kb_ptr)
{
	bound_key = kb_ptr;
}
void Button::setBoundHotkey(Hotkey* hotkey_ptr)
{
	bound_hotkey = hotkey_ptr;
}
void Button::setBoundStickIndex(int* stick_index_ptr)
{
	bound_stick_idx = stick_index_ptr;
}
void Button::setHotkeyIndx(size_t indx)
{
	hotkeyindx = indx;
}
void Button::setBindName(string const& new_name)
{
	 bind_name = new_name;
}
void Button::setIcon(int icon)
{
	icontype = icon;
	if(alDialog && btnType == type::ICON)
		alDialog->d1 = icon;
}
void Button::setText(std::string newText)
{
	// text = std::move(newText);
	// TODO If we don't move, we can enable a hack where we don't reallocate string data.
	// See launcher_dialog.cpp `btn_download_update`.
	text = newText;
}

void Button::setOnPress(std::function<void()> newOnPress)
{
	onPress = std::move(newOnPress);
}

void Button::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void Button::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void Button::calculateSize()
{
	if(btnType == type::ICON)
		setPreferredWidth(3_em);
	else setPreferredWidth(16_px+Size::pixels(gui_text_width(widgFont, text.c_str())));
	Widget::calculateSize();
}

void Button::applyFont(FONT* newFont)
{
	if(alDialog && btnType != type::ICON)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

void Button::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	switch(btnType)
	{
		case type::BASIC:
			alDialog = runner.push(shared_from_this(), DIALOG {
				newGUIProc<jwin_button_proc>,
				x, y, getWidth(), getHeight(),
				fgColor, bgColor,
				getAccelKey(text),
				getFlags(),
				0, 0, // d1, d2
				text.data(), widgFont, nullptr // dp, dp2, dp3
			});
			break;
		case type::ICON:
			alDialog = runner.push(shared_from_this(), DIALOG {
				newGUIProc<jwin_iconbutton_proc>,
				x, y, getWidth(), getHeight(),
				fgColor, bgColor,
				0,
				getFlags(),
				icontype, 0, // d1, d2
				nullptr, nullptr, nullptr // dp, dp2, dp3
			});
			break;
		case type::BIND_KB:
		case type::BIND_HOTKEY:
		case type::BIND_KB_CLEAR:
		case type::BIND_HOTKEY_CLEAR:
			alDialog = runner.push(shared_from_this(), DIALOG {
				newGUIProc<d_kbutton_proc>,
				x, y, getWidth(), getHeight(),
				fgColor, bgColor,
				getAccelKey(text),
				getFlags(),
				0, 0, // d1, d2
				text.data(), widgFont, this // dp, dp2, dp3
			});
			break;
		case type::BIND_JOYKEY:
		case type::BIND_JOYSTICK:
			alDialog = runner.push(shared_from_this(), DIALOG {
				newGUIProc<d_joybutton_proc>,
				x, y, getWidth(), getHeight(),
				fgColor, bgColor,
				getAccelKey(text),
				getFlags(),
				0, 0, // d1, d2
				text.data(), widgFont, this // dp, dp2, dp3
			});
			break;
	}
}

int32_t Button::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCLICK);
	// jwin_button_proc doesn't seem to allow for a non-toggle button...
	alDialog->flags &= ~D_SELECTED;
	
	int ret = -1;
	if (onPress)
	{
		onPress();
		ret = D_REDRAWME;
	}
	if(message >= 0)
		sendMessage(message, MessageArg::none);
	return ret;
}

}
