/**
 * Control Scheme system
 *
 * The scheme named 'Default' is non-editable, and is reset
 *     to default values every time controls are loaded.
 * If no schemes exist (i.e. `controls.cfg` is empty, or does not exist),
 *     then the old control bindings from `[Controls]` in `zc.cfg` are
 *     loaded into a new 'Custom' scheme, which is automatically set
 *     as the globally active control scheme.
 * The "Global" control scheme is used while on the save select screen, and while
 *     playing any quest if the quest isn't set to use a specific scheme.
 * While a quest is running, a quest-specific scheme can be set in the controls
 *     dialog. If set to anything but `(None)`, this takes priority for that quest.
 *     Per-quest config is saved similarly to `Test Init Data`, saved as a config
 *     key per quest in `zc.cfg` `[Controls]`.
 *
 * `controls.cfg` uses Allegro 5 config system (allegro 4 config system has no way to
 *     erase section headers, which made it so there wasn't a good way to 'delete' a scheme)
 *     This system is NOT hooked up to `base_cfg`, so supplying a `controls.cfg` there will
 *     have no effect.
 */
#include "control_scheme.h"
#include "zconfig.h"
#include <cctype>
#include <fmt/format.h>
#include "base/util.h"
#include "zc/replay.h"
#include "zalleg/zalleg.h"

// the key used to store quest-specific schemes in the config
// if empty, no quest is active
string quest_control_path;

// in the order of 'enum controls'
static const char* btn_names[NUM_SCHEME_KEYS] = {
	"up", "down", "left", "right",
	"a", "b", "s", "l", "r", "p",
	"ex1", "ex2", "ex3", "ex4",
};
static const char* cheatmod_names[4] = {
	"key_cheatmod_a1", "key_cheatmod_a2", "key_cheatmod_b1", "key_cheatmod_b2"
};

const control_scheme default_control_scheme;
const string quest_control_prefix = "qst_controls__"; // prefix for quest_control_path

zc_a5_cfg control_config; // `controls.cfg` config manager

// The loaded schemes
std::map<string, control_scheme> control_schemes;

// The active scheme(s). If 'quest_control_scheme_name' is nullopt, 'global_control_scheme_name' is used.
string global_control_scheme_name;
optional<string> quest_control_scheme_name;
optional<string> gamepad_control_scheme_name; // assigned to the active gamepad; see poll_gamepad_scheme

string active_control_scheme_name;
control_scheme const* active_control_scheme = nullptr;
control_scheme replay_control_scheme; // Specially used for replay input handling
static char ctrl_sect[] = "Controls"; // The section in `zc.cfg` to store which schemes are assigned

void update_quest_control_path(string const& path)
{
	if (path.empty())
	{
		quest_control_path.clear();
		quest_control_scheme_name = nullopt;
	}
	else
	{
		quest_control_path = fmt::format("{}{}", quest_control_prefix, qst_cfg_header_from_path(path));
		
		const char* ptr = zc_get_config(ctrl_sect, quest_control_path.c_str(), nullptr);
		if (zc_cfg_defaulted || !ptr || !ptr[0])
			quest_control_scheme_name = nullopt;
		else
			quest_control_scheme_name = string(ptr);
	}
	refresh_control_scheme();
}

void refresh_control_scheme()
{
	if (replay_is_replaying()) // replays mess with controls, so need a separate scheme to modify
	{
		active_control_scheme = &replay_control_scheme;
		return;
	}
	if (!activate_control_scheme(quest_control_scheme_name ? *quest_control_scheme_name :
		gamepad_control_scheme_name ? *gamepad_control_scheme_name : global_control_scheme_name))
	{
		// If the quest-specific (or gamepad) scheme fails, fallback on the global scheme
		if (!activate_control_scheme(global_control_scheme_name))
		{
			// If both assigned schemes fail, fallback on the 'Default' scheme
			if (!activate_control_scheme(DEFAULT_CONTROL_SCHEME_NAME))
			{
				// If the 'Default' scheme fails, forcibly create it.
				default_control_scheme.save_to_section(DEFAULT_CONTROL_SCHEME_NAME);
				control_schemes[DEFAULT_CONTROL_SCHEME_NAME] = default_control_scheme;
				// Must succeed, as it was just created.
				activate_control_scheme(DEFAULT_CONTROL_SCHEME_NAME);
			}
		}
	}
}

void save_schemes()
{
	zc_set_config(ctrl_sect, "global_control_scheme", global_control_scheme_name.c_str());
	if (!quest_control_path.empty())
		zc_set_config(ctrl_sect, quest_control_path.c_str(), quest_control_scheme_name ? quest_control_scheme_name->c_str() : nullptr);
}

void load_control_schemes()
{
	global_control_scheme_name = zc_get_config(ctrl_sect, "global_control_scheme", DEFAULT_CONTROL_SCHEME_NAME);
	quest_control_scheme_name = nullopt;
	
	control_config.re_load("controls.cfg");
	
	control_schemes.clear();
	auto it = control_config.iterate_sections(); // each section is a scheme
	while (it)
	{
		string name = *it.next();
		if (name == DEFAULT_CONTROL_SCHEME_NAME) continue; // skip loading the Default as it is always reset
		control_schemes.emplace(name, name);
	}
	
	if (control_schemes.empty()) // no controls, load the old config
	{
		control_scheme scheme;
		scheme.load_from_old_section(ctrl_sect);
		control_config.add_comment("Custom", "Auto-generated from old config file");
		scheme.save_to_section("Custom");
		control_schemes["Custom"] = scheme;
		global_control_scheme_name = "Custom";
		save_schemes();
	}
	
	// Nuke the 'Default' scheme from the config, then re-add it.
	// Force-resets any changes to it made manually.
	control_config.remove_section(DEFAULT_CONTROL_SCHEME_NAME);
	control_config.add_comment(DEFAULT_CONTROL_SCHEME_NAME, "Non-modifiable. Changes are automatically reset.");
	default_control_scheme.save_to_section(DEFAULT_CONTROL_SCHEME_NAME);
	control_schemes[DEFAULT_CONTROL_SCHEME_NAME] = default_control_scheme;
	
	// Ensure a valid scheme is loaded (otherwise 'active_control_scheme' might cause null deref)
	refresh_control_scheme();
}

void cleanup_control_schemes() // make sure this cleans up before allegro exits?
{
	control_config.destroy();
}

// ---- Per-gamepad scheme assignment ----
// When the active joystick is a recognized gamepad, a scheme is auto-created
// for it (named after the controller) and assigned to its identity (GUID, or
// name when the driver reports no GUID); the assignment is remembered as
// `[Controls] gamepad__<identity>` in zc.cfg. Scheme priority is
// quest-specific > gamepad > global.

static string poll_last_identity = "\n"; // impossible value; the first poll always evaluates

// Stable identity for a controller, used in the config key that remembers its
// scheme assignment. Normally the GUID; some drivers (e.g. allegro's native
// XInput driver on Windows) report an all-zero GUID, so fall back to the
// controller name to keep distinct pads distinct.
static string joystick_identity_str(ALLEGRO_JOYSTICK* joy)
{
	ALLEGRO_JOYSTICK_GUID guid = al_get_joystick_guid(joy);
	bool guid_is_zero = true;
	for (size_t i = 0; i < sizeof(guid.val); i++)
		if (guid.val[i]) guid_is_zero = false;
	if (!guid_is_zero)
	{
		string ret;
		for (size_t i = 0; i < sizeof(guid.val); i++)
			ret += fmt::format("{:02x}", guid.val[i]);
		return ret;
	}
	const char* name = al_get_joystick_name(joy);
	string ret = name && name[0] ? name : "unknown";
	// The name becomes part of a config key, so keep it to safe characters.
	for (char& c : ret)
		if (!isalnum((unsigned char)c))
			c = '_';
	return ret;
}

// From allegro's SDL joystick driver (src/sdl/sdl_joystick.c on the fork): the
// joystick's SDL_GameControllerType as an int, or -1 if the joystick is not
// an SDL game controller (e.g. a native joystick driver is selected).
extern "C" int _al_sdl_joystick_controller_type(ALLEGRO_JOYSTICK* joy);
// Also from the fork's driver: 1 if SDL chose a label-based mapping for the
// pad (see gamepad_reported_by_labels), 0 if positional, -1 as above.
extern "C" int _al_sdl_joystick_mapping_uses_labels(ALLEGRO_JOYSTICK* joy);

// Values from SDL_GameControllerType.
static bool is_playstation_type(int type)
{
	return type == 3 // SDL_CONTROLLER_TYPE_PS3
		|| type == 4 // SDL_CONTROLLER_TYPE_PS4
		|| type == 7; // SDL_CONTROLLER_TYPE_PS5
}

// Nintendo's letters go A east, B south, X north, Y west (the SNES
// arrangement), mirroring Xbox's A south, B east, X west, Y north.
static bool has_nintendo_labels(ALLEGRO_JOYSTICK* joy)
{
	int type = _al_sdl_joystick_controller_type(joy);
	if (is_playstation_type(type))
		return false;
	if (type == 5 || (type >= 11 && type <= 13)) // Switch Pro, Joy-Cons
		return true;
	// A pad SDL maps by label has labels that differ from position, which
	// only Nintendo's do.
	if (_al_sdl_joystick_mapping_uses_labels(joy) == 1)
		return true;

	// SDL's GUID layout: bytes 4-5 hold the vendor id and bytes 8-9 the
	// product id (both little-endian). Through Apple's GameController
	// framework SDL only knows the ids of the pads it recognizes (Xbox,
	// PlayStation, Switch) and reports Apple's own vendor id for the rest,
	// so the name is checked as well.
	ALLEGRO_JOYSTICK_GUID guid = al_get_joystick_guid(joy);
	uint16_t vendor = guid.val[4] | (guid.val[5] << 8);
	uint16_t product = guid.val[8] | (guid.val[9] << 8);
	const char* name = al_get_joystick_name(joy);
	string lname = name ? name : "";
	for (char& c : lname)
		c = tolower((unsigned char)c);
	// 8BitDo: nearly all their pads use Nintendo labels. The Ultimate line
	// copies Xbox's (Ultimate Wired, Ultimate Wireless, Ultimate 2C,
	// Ultimate 2 Wireless), except its Switch-oriented Bluetooth models.
	if (vendor == 0x2dc8 || lname.find("8bitdo") != string::npos)
	{
		if (product == 0x3011 || product == 0x3013 || product == 0x301b)
			return false;
		if (lname.find("ultimate") != string::npos && lname.find("bluetooth") == string::npos)
			return false;
		return true;
	}
	return false;
}

// Whether SDL numbers `joy`'s face buttons by printed label rather than by
// position, so that SDL "A" is the east button of a Nintendo-labeled pad.
// SDL does that for such pads through its HIDAPI drivers and Apple's
// GameController framework, which apply
// SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS themselves, and through a
// generic backend (evdev, DirectInput, IOKit) whenever its controller
// database has a label-based mapping for the pad, which it does for the
// Switch Pro, the Joy-Cons and most 8BitDo models. A Nintendo-labeled pad
// the database does not know (e.g. the 8BitDo Micro) falls back to a
// positional mapping there: the same presses give a,b,x,y through
// GameController and b,a,y,x through IOKit.
//
// To test the positional case on macOS with such a pad, launch with the
// environment variable SDL_JOYSTICK_MFI=0 so SDL bypasses the GameController
// framework and takes the pad through IOKit. The auto-created scheme is
// remembered, so delete the pad's scheme in controls.cfg (or clear its
// assignment in the Control Schemes dialog) first to get a freshly generated
// default.
static bool gamepad_reported_by_labels(ALLEGRO_JOYSTICK* joy)
{
	if (!has_nintendo_labels(joy))
		return false;
	// SDL's GUID layout: byte 14 holds the backend signature ('h' HIDAPI,
	// 'm' GameController; 0 for the generic backends).
	uint8_t backend = al_get_joystick_guid(joy).val[14];
	if (backend == 'h' || backend == 'm')
		return true;
	return _al_sdl_joystick_mapping_uses_labels(joy) == 1;
}

// SDL numbers the face buttons by position (south, east, west, north are
// SDL's A, B, X, Y, which is also how Xbox labels them), except for the
// Nintendo-labeled pads it reports by label (see gamepad_reported_by_labels).
static gamepad_face_buttons get_gamepad_face_buttons(ALLEGRO_JOYSTICK* joy)
{
	if (joy && gamepad_reported_by_labels(joy))
		return {2, 1, 4, 3};
	return {1, 2, 3, 4};
}

void set_gamepad_face_layout(control_scheme& scheme, ALLEGRO_JOYSTICK* joy, gamepad_face_layout layout)
{
	set_gamepad_face_layout(scheme, get_gamepad_face_buttons(joy), layout);
}

void set_gamepad_face_layout(control_scheme& scheme, gamepad_face_buttons const& face, gamepad_face_layout layout)
{
	int* b = scheme.btns;
	if (layout == gamepad_face_layout::nintendo)
	{
		b[btnA] = face.east;
		b[btnB] = face.south;
		b[btnEx1] = face.north; // X
		b[btnEx2] = face.west;  // Y
	}
	else
	{
		b[btnA] = face.south;
		b[btnB] = face.east;
		b[btnEx1] = face.west;  // X
		b[btnEx2] = face.north; // Y
	}
}

const char* gamepad_button_label(ALLEGRO_JOYSTICK* joy, int btn)
{
	// The driver names every gamepad's buttons after the Xbox controller's.
	if (!joy)
		return nullptr;
	int type = _al_sdl_joystick_controller_type(joy);

	if (has_nintendo_labels(joy))
	{
		// A positionally numbered pad's driver name is the letter at that
		// position on an Xbox pad, which is the other letter here.
		if (!gamepad_reported_by_labels(joy))
			switch (btn)
			{
				case 1: return "B"; // south
				case 2: return "A"; // east
				case 3: return "Y"; // west
				case 4: return "X"; // north
			}
		if (type == 5 || type == 13) // Switch Pro, Joy-Con pair
			switch (btn)
			{
				case 5: return "L"; // left shoulder
				case 6: return "R"; // right shoulder
				case 7: return "Minus"; // back
				case 8: return "Plus"; // start
				case 9: return "Home"; // guide
				case 12: return "ZL"; // left trigger
				case 13: return "ZR"; // right trigger
			}
		return nullptr;
	}

	// PlayStation pads mark the face buttons with symbols instead of letters,
	// and call their other buttons something else too.
	if (!is_playstation_type(type))
		return nullptr;

	switch (btn)
	{
		case 1: return "Cross"; // A
		case 2: return "Circle"; // B
		case 3: return "Square"; // X
		case 4: return "Triangle"; // Y
		case 5: return "L1"; // left shoulder
		case 6: return "R1"; // right shoulder
		case 7: return type == 3 ? "Select" : type == 4 ? "Share" : "Create"; // back
		case 8: return type == 3 ? "Start" : "Options"; // start
		case 9: return "PS Button"; // guide
		case 10: return "L3"; // left thumb
		case 11: return "R3"; // right thumb
		case 12: return "L2"; // left trigger
		case 13: return "R2"; // right trigger
	}
	return nullptr;
}

static control_scheme make_gamepad_default_scheme(ALLEGRO_JOYSTICK* joy)
{
	// Buttons are 1-based (see joybtn); allegro gamepad button N is N+1 here.
	// The a5_joystick shim appends synthetic buttons after the 11 gamepad
	// buttons: 12/13 = left/right trigger, 14..17 = dpad up/down/left/right.
	//
	// Start from the active scheme so keyboard and cheat bindings carry over;
	// every gamepad-related field is overwritten with gamepad defaults below.
	control_scheme scheme = active_control_scheme ? *active_control_scheme : control_scheme();
	scheme.analog_movement = true;
	int* b = scheme.btns;
	b[btnUp] = 14; b[btnDown] = 15; b[btnLeft] = 16; b[btnRight] = 17;
	// Face buttons: quests hardcode the engine's A/B/X/Y button names into
	// their subscreens, so put each action on the button printed with its
	// letter. PlayStation pads have no letters, so they get the SNES
	// arrangement. Pads that are neither PlayStation nor Nintendo-labeled
	// are assumed to copy the Xbox labels.
	bool nintendo = is_playstation_type(_al_sdl_joystick_controller_type(joy)) || has_nintendo_labels(joy);
	set_gamepad_face_layout(scheme, joy, nintendo ? gamepad_face_layout::nintendo : gamepad_face_layout::xbox);
	b[btnS] = 8;    // start
	b[btnL] = 5;    // left shoulder
	b[btnR] = 6;    // right shoulder
	b[btnP] = 7;    // back/select
	b[btnEx3] = 12; // left trigger
	b[btnEx4] = 13; // right trigger
	scheme.btn_menu = 9; // guide
	// Move with the left thumb stick; the dpad works via its buttons above.
	memcpy(scheme.stick_data, default_control_scheme.stick_data, sizeof(scheme.stick_data));
	return scheme;
}

void poll_gamepad_scheme()
{
	zalleg_log_joysticks_if_changed();

	if (replay_is_replaying())
		return;

	// Which joystick to identify comes from the scheme the gamepad assignment
	// overrides (quest-specific or global), not from the active scheme: an
	// assigned scheme that itself points at a different pad would otherwise
	// change which identity gets evaluated next poll, and two such schemes
	// pointing at each other would alternate every frame.
	int index = 0;
	string const& base_name = quest_control_scheme_name ? *quest_control_scheme_name : global_control_scheme_name;
	if (auto it = control_schemes.find(base_name); it != control_schemes.end())
		index = it->second.joystick_index;
	else if (active_control_scheme)
		index = active_control_scheme->joystick_index;
	ALLEGRO_JOYSTICK* joy = index < al_get_num_joysticks() ? al_get_joystick(index) : nullptr;

	string identity = joy ? joystick_identity_str(joy) : "";
	if (identity == poll_last_identity)
		return;
	poll_last_identity = identity;

	optional<string> prev = gamepad_control_scheme_name;
	gamepad_control_scheme_name = nullopt;
	if (!identity.empty())
	{
		string key = fmt::format("gamepad__{}", identity);
		const char* assigned = zc_get_config(ctrl_sect, key.c_str(), nullptr);
		if (assigned && assigned[0] && control_schemes.contains(assigned))
			gamepad_control_scheme_name = string(assigned);
		else if (al_get_joystick_type(joy) == ALLEGRO_JOYSTICK_TYPE_GAMEPAD)
		{
			// First time seeing this gamepad: give it a scheme named after the
			// controller, creating one with gamepad defaults if the name is
			// unused. Schemes carried over from older versions are NOT reused
			// here even if customized: their button numbers were tuned to the
			// old raw driver layouts, which the SDL driver renumbered.
			// (Keyboard bindings do carry over - see
			// make_gamepad_default_scheme.) The same controller model shares
			// one scheme, which also keeps a pad's bindings when it switches
			// between USB and Bluetooth (those get different GUIDs but the
			// same name).
			const char* joy_name = al_get_joystick_name(joy);
			string name = joy_name && joy_name[0] ? joy_name : "Gamepad";
			if (!control_schemes.contains(name))
			{
				control_schemes[name] = make_gamepad_default_scheme(joy);
				control_schemes[name].save_to_section(name);
			}
			zc_set_config(ctrl_sect, key.c_str(), name.c_str());
			gamepad_control_scheme_name = name;
		}
	}
	if (gamepad_control_scheme_name != prev)
		refresh_control_scheme();
}

static optional<string> gamepad_assignment_key(int joy_index)
{
	if (joy_index < 0 || joy_index >= al_get_num_joysticks())
		return nullopt;

	ALLEGRO_JOYSTICK* joy = al_get_joystick(joy_index);
	if (!joy)
		return nullopt;

	return fmt::format("gamepad__{}", joystick_identity_str(joy));
}

optional<string> get_gamepad_assigned_scheme(int joy_index)
{
	auto key = gamepad_assignment_key(joy_index);
	if (!key)
		return nullopt;

	const char* assigned = zc_get_config(ctrl_sect, key->c_str(), nullptr);
	if (assigned && assigned[0] && control_schemes.contains(assigned))
		return string(assigned);

	return nullopt;
}

void set_gamepad_assigned_scheme(int joy_index, string const& name)
{
	auto key = gamepad_assignment_key(joy_index);
	if (!key)
		return;

	// An empty name clears the assignment; the next poll re-assigns
	// automatically.
	zc_set_config(ctrl_sect, key->c_str(), name.empty() ? nullptr : name.c_str());
	poll_last_identity = "\n"; // force re-evaluation
	poll_gamepad_scheme();
}

bool activate_control_scheme(string const& name)
{
	if (!control_schemes.contains(name))
		return false;
	active_control_scheme_name = name;
	active_control_scheme = replay_is_replaying() ? &replay_control_scheme : &control_schemes[name];
	return true;
}
bool create_control_scheme(string const& name)
{
	if (control_schemes.contains(name))
		return false;
	control_schemes[name] = default_control_scheme;
	control_schemes[name].save_to_section(name);
	return true;
}
bool delete_control_scheme(string const& name)
{
	if (!control_schemes.contains(name))
		return false;
	if (name == DEFAULT_CONTROL_SCHEME_NAME) return false;
	control_schemes.erase(name);
	if (quest_control_scheme_name && *quest_control_scheme_name == name)
		quest_control_scheme_name = nullopt;
	if (gamepad_control_scheme_name && *gamepad_control_scheme_name == name)
		gamepad_control_scheme_name = nullopt;
	if (global_control_scheme_name == name)
	{
		string newname = DEFAULT_CONTROL_SCHEME_NAME;
		if (!control_schemes.empty() && !control_schemes.contains(DEFAULT_CONTROL_SCHEME_NAME))
		{
			auto [s_name, s_obj] = *(control_schemes.begin());
			newname = s_name;
		}
		global_control_scheme_name = newname;
	}
	control_config.remove_section(name.c_str());
	
	const char** strings = nullptr;
	int entry_count = list_config_entries(ctrl_sect, &strings);
	for (int q = 0; q < entry_count; ++q)
	{
		if (string(zc_get_config(ctrl_sect, strings[q], "")) == name)
			zc_set_config(ctrl_sect, strings[q], nullptr);
	}
	free_config_entries(&strings);
	return true;
}
bool rename_control_scheme(string const& oldname, string const& newname)
{
	if (oldname == DEFAULT_CONTROL_SCHEME_NAME)
		return false;
	if (!control_schemes.contains(oldname))
		return false;
	if (control_schemes.contains(newname))
		return false;
	control_schemes[newname] = control_schemes[oldname];
	if (active_control_scheme_name == oldname || active_control_scheme == &control_schemes[oldname])
		activate_control_scheme(newname);
	control_schemes.erase(oldname);
	
	control_schemes[newname].save_to_section(newname);
	
	control_config.remove_section(oldname.c_str());
	
	// convert saved control sets
	if (quest_control_scheme_name && *quest_control_scheme_name == oldname)
		quest_control_scheme_name = newname;
	if (gamepad_control_scheme_name && *gamepad_control_scheme_name == oldname)
		gamepad_control_scheme_name = newname;
	if (global_control_scheme_name == oldname)
		global_control_scheme_name = newname;
	
	const char** strings = nullptr;
	int entry_count = list_config_entries(ctrl_sect, &strings);
	for (int q = 0; q < entry_count; ++q)
	{
		if (string(zc_get_config(ctrl_sect, strings[q], "")) == oldname)
			zc_set_config(ctrl_sect, strings[q], newname.c_str());
	}
	free_config_entries(&strings);
	return true;
}

control_scheme::control_scheme()
{
	memset(cheatkeys, 0, sizeof(cheatkeys));
	cheatkeys[Cheat::Life][0] = KEY_H;
	cheatkeys[Cheat::Life][1] = KEY_ASTERISK;
	cheatkeys[Cheat::Magic][0] = KEY_M;
	cheatkeys[Cheat::Magic][1] = KEY_SLASH_PAD;
	cheatkeys[Cheat::Rupies][0] = KEY_R;
	cheatkeys[Cheat::Bombs][0] = KEY_B;
	cheatkeys[Cheat::Arrows][0] = KEY_A;
	cheatkeys[Cheat::Clock][0] = KEY_I;
	cheatkeys[Cheat::Walls][0] = KEY_F11;
	cheatkeys[Cheat::Fast][0] = KEY_Q;
	cheatkeys[Cheat::Light][0] = KEY_L;
	cheatkeys[Cheat::IgnoreSideView][0] = KEY_V;
	cheatkeys[Cheat::Kill][0] = KEY_K;
	cheatkeys[Cheat::GoTo][0] = KEY_G;
	cheatkeys[Cheat::TrigSecrets][0] = KEY_S;
	cheatkeys[Cheat::ShowL0][0] = KEY_0;
	cheatkeys[Cheat::ShowL1][0] = KEY_1;
	cheatkeys[Cheat::ShowL2][0] = KEY_2;
	cheatkeys[Cheat::ShowL3][0] = KEY_3;
	cheatkeys[Cheat::ShowL4][0] = KEY_4;
	cheatkeys[Cheat::ShowL5][0] = KEY_5;
	cheatkeys[Cheat::ShowL6][0] = KEY_6;
	cheatkeys[Cheat::ShowFFC][0] = KEY_7;
	cheatkeys[Cheat::ShowSprites][0] = KEY_8;
	cheatkeys[Cheat::ShowWalkability][0] = KEY_W;
	cheatkeys[Cheat::ShowEffects][0] = KEY_E;
	cheatkeys[Cheat::ShowOverhead][0] = KEY_O;
	cheatkeys[Cheat::ShowPushblock][0] = KEY_P;
	cheatkeys[Cheat::ShowHitbox][0] = KEY_C;
	cheatkeys[Cheat::ShowFFCScripts][0] = KEY_F;
	
	// Like btns, sticks follow the gamepad model, where stick 0 is the dpad:
	// move with the left thumb stick, and read each stick's Y from its second
	// axis.
	memset(stick_data, 0, sizeof(stick_data));
	for (int stick = 0; stick < control_scheme::num_sticks; stick++)
		stick_data[stick][control_scheme::axis_y][control_scheme::data_axis] = 1;
	for (int axis = 0; axis < control_scheme::num_axes; axis++)
	{
		stick_data[control_scheme::stick_1][axis][control_scheme::data_stick] = ALLEGRO_GAMEPAD_STICK_LEFT_THUMB;
		stick_data[control_scheme::stick_2][axis][control_scheme::data_stick] = ALLEGRO_GAMEPAD_STICK_RIGHT_THUMB;
	}
}
static const string stick_id1_strs[] = {"1", "2"};
static const string stick_id2_strs[] = {"x", "y"};
static const string stick_id3_strs[] = {"stick", "axis", "offset"};
control_scheme::control_scheme(string const& scheme_name) : control_scheme()
{
	load_from_section(scheme_name);
}
control_scheme::control_scheme(const char* scheme_name) : control_scheme()
{
	load_from_section(scheme_name);
}

void control_scheme::simplify()
{
	for(size_t q = 1; q < Cheat::Last; ++q) // collapse cheats with a 2nd bind but no 1st bind
	{
		if(cheatkeys[q][1] && !cheatkeys[q][0])
		{
			cheatkeys[q][0] = cheatkeys[q][1];
			cheatkeys[q][1] = 0;
		}
	}
	for (int id1 = 0; id1 < control_scheme::num_sticks; ++id1) // match the X/Y sticks
		stick_data[id1][control_scheme::axis_y][control_scheme::data_stick] = stick_data[id1][control_scheme::axis_x][control_scheme::data_stick];
}
bool control_scheme::save() const
{
	for (auto& [key, scheme] : control_schemes)
	{
		if (&scheme == this)
		{
			save_to_section(key);
			return true;
		}
	}
	return false;
}
// Schemes saved before the stick defaults were fixed read stick 2's Y from its
// first axis, the same one as its X. No binding UI sets axes, and reading both
// from one axis is never intended, so point Y back at the second axis.
static void repair_stick_axes(control_scheme& scheme)
{
	for (int stick = 0; stick < control_scheme::num_sticks; stick++)
	{
		auto& data = scheme.stick_data[stick];
		if (data[control_scheme::axis_x][control_scheme::data_axis] == data[control_scheme::axis_y][control_scheme::data_axis])
			data[control_scheme::axis_y][control_scheme::data_axis] = data[control_scheme::axis_x][control_scheme::data_axis] ? 0 : 1;
	}
}

void control_scheme::load_from_section(string const& scheme_name)
{
	load_from_section(scheme_name.c_str());
}
void control_scheme::load_from_section(const char* scheme_name)
{
	for (int q = 0; q < NUM_SCHEME_KEYS; ++q)
	{
		keys[q] = control_config.get_config_int(scheme_name, fmt::format("key_{}", btn_names[q]).c_str()).value_or(default_control_scheme.keys[q]);
		btns[q] = control_config.get_config_int(scheme_name, fmt::format("btn_{}", btn_names[q]).c_str()).value_or(default_control_scheme.btns[q]);
	}
	btn_menu = control_config.get_config_int(scheme_name,"btn_menu").value_or(default_control_scheme.btn_menu);
	
	analog_movement = control_config.get_config_int(scheme_name,"analog_movement").value_or(default_control_scheme.analog_movement ? 1 : 0);
	joystick_index = control_config.get_config_int(scheme_name,"joystick_index").value_or(0);
	for (int id1 = 0; id1 < control_scheme::num_sticks; ++id1)
		for (int id2 = 0; id2 < control_scheme::num_axes; ++id2)
			for(int id3 = 0; id3 < control_scheme::num_data; ++id3)
			{
				stick_data[id1][id2][id3] = control_config.get_config_int(scheme_name,
					fmt::format("js_stick_{}_{}_{}", stick_id1_strs[id1], stick_id2_strs[id2],
					stick_id3_strs[id3]).c_str()).value_or(default_control_scheme.stick_data[id1][id2][id3]);
				if (id3 == 2)
					stick_data[id1][id2][id3] = (stick_data[id1][id2][id3] ? 128 : 0);
			}
	
	for (int q = 0; q < 4; ++q)
		cheat_modifier_keys[q] = control_config.get_config_int(scheme_name, cheatmod_names[q]).value_or(default_control_scheme.cheat_modifier_keys[q]);
	
	for(size_t q = 1; q < Cheat::Last; ++q)
	{
		if(!bindable_cheat((Cheat)q)) continue;
		std::string cheatname = cheat_to_string((Cheat)q);
		util::lowerstr(cheatname);
		cheatkeys[q][0] = control_config.get_config_int(scheme_name,fmt::format("key_cheat_{}_main", cheatname).c_str()).value_or(default_control_scheme.cheatkeys[q][0]);
		cheatkeys[q][1] = control_config.get_config_int(scheme_name,fmt::format("key_cheat_{}_alt", cheatname).c_str()).value_or(default_control_scheme.cheatkeys[q][1]);
	}
	
	repair_stick_axes(*this);
	if((uint32_t)joystick_index >= MAX_JOYSTICKS)
		joystick_index = 0;
}
void control_scheme::load_from_old_section(const char* section_name)
{
	for (int q = 0; q < NUM_SCHEME_KEYS; ++q)
	{
		keys[q] = zc_get_config(section_name, fmt::format("key_{}", btn_names[q]).c_str(), default_control_scheme.keys[q]);
		btns[q] = zc_get_config(section_name, fmt::format("btn_{}", btn_names[q]).c_str(), default_control_scheme.btns[q]);
	}
	btn_menu = zc_get_config(section_name,"btn_m",default_control_scheme.btn_menu);
	
	analog_movement = zc_get_config(section_name,"analog_movement", default_control_scheme.analog_movement ? 1 : 0);
	joystick_index = zc_get_config(section_name,"joystick_index",0);
	for (int id1 = 0; id1 < control_scheme::num_sticks; ++id1)
		for (int id2 = 0; id2 < control_scheme::num_axes; ++id2)
			for(int id3 = 0; id3 < control_scheme::num_data; ++id3)
			{
				stick_data[id1][id2][id3] = zc_get_config(section_name,
					fmt::format("js_stick_{}_{}_{}", stick_id1_strs[id1], stick_id2_strs[id2], stick_id3_strs[id3]).c_str(),
					default_control_scheme.stick_data[id1][id2][id3]);
				if (id3 == 2)
					stick_data[id1][id2][id3] = (stick_data[id1][id2][id3] ? 128 : 0);
			}
	
	for (int q = 0; q < 4; ++q)
		cheat_modifier_keys[q] = zc_get_config(section_name, cheatmod_names[q], default_control_scheme.cheat_modifier_keys[q]);
	
	for(size_t q = 1; q < Cheat::Last; ++q)
	{
		if(!bindable_cheat((Cheat)q)) continue;
		std::string cheatname = cheat_to_string((Cheat)q);
		util::lowerstr(cheatname);
		cheatkeys[q][0] = zc_get_config(section_name,fmt::format("key_cheat_{}_main", cheatname).c_str(),default_control_scheme.cheatkeys[q][0]);
		cheatkeys[q][1] = zc_get_config(section_name,fmt::format("key_cheat_{}_alt", cheatname).c_str(),default_control_scheme.cheatkeys[q][1]);
	}
	
	repair_stick_axes(*this);
	if((uint32_t)joystick_index >= MAX_JOYSTICKS)
		joystick_index = 0;
}
void control_scheme::save_to_section(string const& scheme_name) const
{
	save_to_section(scheme_name.c_str());
}
void control_scheme::save_to_section(const char* scheme_name) const
{
	for (int q = 0; q < NUM_SCHEME_KEYS; ++q)
	{
		control_config.set_config(scheme_name, fmt::format("key_{}", btn_names[q]).c_str(), keys[q]);
		control_config.set_config(scheme_name, fmt::format("btn_{}", btn_names[q]).c_str(), btns[q]);
	}
	control_config.set_config(scheme_name,"btn_menu",btn_menu);
	
	control_config.set_config(scheme_name,"analog_movement", analog_movement ? 1 : 0);
	control_config.set_config(scheme_name,"joystick_index", joystick_index);
	for (int id1 = 0; id1 < control_scheme::num_sticks; ++id1)
		for (int id2 = 0; id2 < control_scheme::num_axes; ++id2)
			for(int id3 = 0; id3 < control_scheme::num_data; ++id3)
			{
				control_config.set_config(scheme_name,
					fmt::format("js_stick_{}_{}_{}", stick_id1_strs[id1], stick_id2_strs[id2], stick_id3_strs[id3]).c_str(),
					stick_data[id1][id2][id3]);
			}
   
	for (int q = 0; q < 4; ++q)
		control_config.set_config(scheme_name, cheatmod_names[q], cheat_modifier_keys[q]);
	for(size_t q = 1; q < Cheat::Last; ++q)
	{
		if(!bindable_cheat((Cheat)q)) continue;
		std::string cheatname = cheat_to_string((Cheat)q);
		util::lowerstr(cheatname);
		control_config.set_config(scheme_name,fmt::format("key_cheat_{}_main", cheatname).c_str(),cheatkeys[q][0]);
		control_config.set_config(scheme_name,fmt::format("key_cheat_{}_alt", cheatname).c_str(),cheatkeys[q][1]);
	}
	control_config.save();
}

