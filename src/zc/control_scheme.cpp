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
#include <fmt/format.h>
#include "base/util.h"
#include "zc/replay.h"

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
	if (!activate_control_scheme(quest_control_scheme_name ? *quest_control_scheme_name : global_control_scheme_name))
	{
		// If the quest-specific scheme fails, fallback on the global scheme
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
	
	memset(stick_data, 0, sizeof(stick_data));
	
	// set y axis to second axis
	stick_data[control_scheme::stick_1][control_scheme::axis_y][control_scheme::data_axis] = 1;
	stick_data[control_scheme::stick_1][control_scheme::axis_y][control_scheme::data_axis] = 1;
	// set second stick to second stick
	stick_data[control_scheme::stick_2][control_scheme::axis_x][control_scheme::data_stick] = 1;
	stick_data[control_scheme::stick_2][control_scheme::axis_y][control_scheme::data_stick] = 1;
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

