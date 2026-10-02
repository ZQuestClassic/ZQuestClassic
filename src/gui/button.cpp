#include "gui/button.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "gui/jwin_a5.h"
#include "base/render.h"
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
// The gamepad inputs currently held: buttons (1-based), or sticks when binding
// a stick.
static std::vector<int> get_held_inputs(int stick_idx, bool stick)
{
	std::vector<int> held;
	if (stick)
	{
		for(int q = 0; q < joy[stick_idx].num_sticks; ++q)
			if(joystick (stick_idx, q))
				held.push_back(q);
	}
	else
	{
		for(int q = 1; q <= joy[stick_idx].num_buttons; ++q)
			if(joybtn (stick_idx, q))
				held.push_back(q);
	}
	return held;
}
// Names an input with its kind and number, since a gamepad's names can be shared
// between a button and a stick (e.g. "Left Thumb" is both the left stick and its
// click).
static string get_input_name(int stick_idx, bool stick, int q)
{
	const char* name = stick ? joy[stick_idx].stick[q].name : joy[stick_idx].button[q-1].name;
	string ret = fmt::format("{} {}", stick ? "stick" : "button", q);
	if (name && name[0])
		ret += fmt::format(" ({})", name);
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
	zq_push_unfrozen_dialogs(1);

	// The spinner arc renders on its own layer, sized to the popup - the framework
	// clears the bitmap before each render, so only the current arc needs drawing.
	double a = 0.0;
	RenderTreeItem* spinner_rti = add_dlg_layer(popup_x, popup_y, popup_w, popup_h);
	spinner_rti->render_cb = [&](RenderTreeItem*, bool) {
		al_draw_arc(ax - popup_x, ay - popup_y, ar, a, width, a5color(jwin_pal[jcBOXFG]), thickness);
	};

	do
	{
		a = wrap_float(a + aspd, 0.0, 2 * PI);
		spinner_rti->dirty = true;
		update_hw_screen();
	}
	while (!proc());

	while(gui_mouse_b())
		rest(1);
	clear_keybuf();

	remove_dlg_layer(spinner_rti);
	zq_pop_unfrozen_dialogs();
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

	// Inputs already held when the popup opens can't be bound until they are
	// released, since one of them may be what opened it. They are tracked one
	// by one, rather than waiting for every input to be released, so an input
	// that never reads as released (a stuck or noisy button or trigger) can't
	// block binding the others. Any still held after a moment are logged, to
	// help diagnose such a controller.
	poll_joystick();
	std::set<int> ignored;
	for (int q : get_held_inputs(stick_idx, stick))
		ignored.insert(q);
	bool bound = false;
	int frames = 0;
	string logged_str;

	spinner_loop(strs, [&]()
		{
			poll_joystick();
			if (!binding_joystick || !al_get_joystick_active(binding_joystick))
				return true; // gamepad disconnected
			auto held = get_held_inputs(stick_idx, stick);
			auto is_held = [&](int q) { return std::find(held.begin(), held.end(), q) != held.end(); };
			if (bound)
			{
				// Stay open until the inputs pressed while it was open are released
				// (ignored ones held since it opened, like a stuck button, don't
				// count). The dialog underneath turns a held button 0/1 on joystick
				// 0 into a Space key and a held dpad into arrow keys (update_dialog
				// in allegro_legacy's gui.c), which would reopen this popup from the
				// still-focused Bind button, or move focus, the moment it closed.
				return std::all_of(held.begin(), held.end(), [&](int q) { return ignored.contains(q); });
			}
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

			std::erase_if(ignored, [&](int q) { return !is_held(q); });
			for (int q : held)
			{
				if (!ignored.contains(q))
				{
					btn_ref = q;
					bound = true;
					return false;
				}
			}

			if (++frames >= 30)
			{
				string ignored_str;
				for (int q : ignored)
				{
					if (!ignored_str.empty())
						ignored_str += ", ";
					ignored_str += get_input_name(stick_idx, stick, q);
				}
				if (!ignored_str.empty() && ignored_str != logged_str)
				{
					al_trace("Gamepad binding: ignoring input held since the popup opened: %s\n", ignored_str.c_str());
					logged_str = ignored_str;
				}
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
	bool bound = false;
	spinner_loop(strs, [&]()
		{
			if (bound)
			{
				// As in joy_getbtn: a key still held when the popup closes
				// would autorepeat into the dialog underneath.
				poll_keyboard();
				return !key[key_ref];
			}
			while (auto key = get_next_keypress(true))
			{
				if (*key == KEY_ESC)
					return true; // exit
				if (*key >= KEY_F1 && *key <= KEY_F12 && *key != KEY_F11)
					continue; // disallow
				if (*key < 0 || *key > 123)
					continue; // out of range, disallow
				key_ref = *key;
				bound = true;
				return false;
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
	strs.emplace_back("Press any key or mouse button (+mods)");
	if (!title.empty())
		strs.emplace_back(title);
	strs.emplace_back("ESC to cancel");
	// Only a mouse button pressed after the popup opens binds - not one still held
	// from clicking the Bind button.
	int prev_mouse_b = gui_mouse_b();
	spinner_loop(strs, [&]()
		{
			int mb = gui_mouse_b();
			int pressed = mb & ~prev_mouse_b;
			prev_mouse_b = mb;
			for (int button = HOTKEY_MOUSE_FIRST_BUTTON; button <= HOTKEY_MOUSE_LAST_BUTTON; ++button)
			{
				if (pressed & (1 << (button - 1)))
				{
					hkey = HOTKEY_MOUSE_CODE(button);
					modflag = get_mods();
					return true;
				}
			}
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
