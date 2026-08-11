#pragma once

#include "base/general.h"
#include "base/headers.h"
#include "zalleg/zalleg.h"
#include "zc/cheats.h"

struct control_scheme;

extern const control_scheme default_control_scheme;
extern const std::string quest_control_prefix;
extern std::map<string, control_scheme> control_schemes;

extern std::string global_control_scheme_name;
extern optional<std::string> quest_control_scheme_name;
extern optional<std::string> gamepad_control_scheme_name;
extern std::string active_control_scheme_name;
extern control_scheme const* active_control_scheme;
extern control_scheme replay_control_scheme;

#define DEFAULT_CONTROL_SCHEME_NAME "Default"

void update_quest_control_path(string const& path);
void poll_gamepad_scheme();
optional<std::string> get_gamepad_assigned_scheme(int joy_index);
void set_gamepad_assigned_scheme(int joy_index, string const& name);
void refresh_control_scheme();
void save_schemes();
void load_control_schemes();
void cleanup_control_schemes();
bool activate_control_scheme(string const& name);
bool create_control_scheme(string const& name);
bool delete_control_scheme(string const& name);
bool rename_control_scheme(string const& oldname, string const& newname);

#define NUM_SCHEME_KEYS (btnEx4+1)
struct control_scheme
{
	enum scheme_stick_stick
	{
		stick_1, stick_2, num_sticks
	};
	enum scheme_stick_axis
	{
		axis_x, axis_y, num_axes
	};
	enum scheme_stick_data
	{
		data_stick, data_axis, data_offset, num_data
	};
	
	control_scheme();
	control_scheme(string const& section_name);
	control_scheme(const char* section_name);
	
	void simplify();
	bool save() const;
	
	void load_from_section(string const& section_name);
	void load_from_section(const char* section_name);
	void load_from_old_section(const char* section_name);
	void save_to_section(string const& section_name) const;
	void save_to_section(const char* section_name) const;
	
	int joystick_index = 0;
	int btn_menu = 9;
	bool analog_movement = true;
	int cheat_modifier_keys[4] = {
#ifdef ALLEGRO_MACOSX
		KEY_COMMAND, 0,
#else
		KEY_LCONTROL, KEY_RCONTROL,
#endif
		 0, 0,
	};
	int keys[NUM_SCHEME_KEYS] = {
		KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
		KEY_Z, KEY_X, KEY_ENTER, KEY_Q, KEY_W, KEY_SPACE,
		KEY_A, KEY_S, KEY_D, KEY_C,
	};
	// Numbered per the SDL joystick driver's gamepad model, 1-based: 1-11 =
	// A,B,X,Y,LS,RS,Back,Start,Guide,LThumb,RThumb; 12/13 = triggers; 14-17 =
	// dpad. Face buttons bind the engine action to the same-lettered button;
	// see make_gamepad_default_scheme for the full policy.
	int btns[NUM_SCHEME_KEYS] = {
		14, 15, 16, 17,
		1, 2, 8, 5, 6, 7,
		3, 4, 12, 13,
	};
	int stick_data[num_sticks][num_axes][num_data];
	int cheatkeys[Cheat::Last][2];
};

