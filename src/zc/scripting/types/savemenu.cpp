#include "zc/scripting/types/savemenu.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/types/musicdata.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

SaveMenu* checkSaveMenu(int32_t ref, bool skipError)
{
	if(unsigned(ref-1) < NUM_SAVE_MENUS)
	{
		return &QMisc.save_menus[ref-1];
	}
	if(skipError) return nullptr;

	scripting_log_error_with_context("Invalid {} using UID = {}", "save_menu", ref);
	return nullptr;
}

std::optional<int32_t> savemenu_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case SAVEMENU_CURSORTILE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->cursor_tile * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_CURSORCSET:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->cursor_cset * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_BGCOLOR:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->bg_color * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_BGTILE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->bg_tile * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_BGCSET:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->bg_cset * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_BGTW:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->bg_tw * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_BGTH:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->bg_th * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_SFX_CURSOR:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->cursor_sfx * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_SFX_CHOOSE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->choose_sfx * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_HSPACE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->hspace * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_VSPACE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->vspace * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_OPTX:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->opt_x * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_OPTY:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->opt_y * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_TEXT_ALIGN:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->text_align * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_TEXT_BOX_ALIGN:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->textbox_align * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_CLOSE_FRAMES:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->close_frames * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_CLOSE_FLASH_RATE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->close_flash_rate * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_MUSIC:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->music;
			break;
		}
		case SAVEMENU_MIDI:
		{
			ret = -10000L;
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				if (!menu->music)
					ret = 0;
				else if (auto* amus = checkMusic(menu->music))
					ret = convert_to_old_midi_id(amus->midi) * 10000L;
			}
			break;
		}
		case SAVEMENU_NUM_OPTIONS:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = menu->options.size() * 10000L;
			else ret = -10000L;
			break;
		}
		case SAVEMENU_ID:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				ret = ri->savemenuref * 10000L;
			else ret = -10000L;
			break;
		}

		default: return std::nullopt;
	}

	return ret;
}
bool savemenu_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case SAVEMENU_CURSORTILE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->cursor_tile = vbound(value / 10000L, 0, NEWMAXTILES-1);
			break;
		}
		case SAVEMENU_CURSORCSET:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->cursor_cset = vbound(value / 10000L, 0, 0xF);
			break;
		}
		case SAVEMENU_BGCOLOR:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->bg_color = vbound(value / 10000L, 0, 0xFF);
			break;
		}
		case SAVEMENU_BGTILE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->bg_tile = vbound(value / 10000, 0, NEWMAXTILES-1);
			break;
		}
		case SAVEMENU_BGCSET:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->bg_cset = vbound(value / 10000, 0, 0xF);
			break;
		}
		case SAVEMENU_BGTW:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->bg_tw = vbound(value / 10000, 0, 16);
			break;
		}
		case SAVEMENU_BGTH:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->bg_th = vbound(value / 10000, 0, 16);
			break;
		}
		case SAVEMENU_SFX_CURSOR:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->cursor_sfx = vbound(value / 10000L, 0, MAX_SFX);
			break;
		}
		case SAVEMENU_SFX_CHOOSE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->choose_sfx = vbound(value / 10000L, 0, MAX_SFX);
			break;
		}
		case SAVEMENU_HSPACE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->hspace = vbound(value / 10000L, 0, 255);
			break;
		}
		case SAVEMENU_VSPACE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->vspace = vbound(value / 10000L, 0, 255);
			break;
		}
		case SAVEMENU_OPTX:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->opt_x = vbound(value / 10000L, 0, 255);
			break;
		}
		case SAVEMENU_OPTY:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->opt_y = vbound(value / 10000L, 0, 255);
			break;
		}
		case SAVEMENU_TEXT_ALIGN:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->text_align = vbound(value / 10000L, 0, 2);
			break;
		}
		case SAVEMENU_TEXT_BOX_ALIGN:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->textbox_align = vbound(value / 10000L, 0, 2);
			break;
		}
		case SAVEMENU_CLOSE_FRAMES:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->close_frames = vbound(value / 10000L, 0, 65535);
			break;
		}
		case SAVEMENU_CLOSE_FLASH_RATE:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->close_flash_rate = vbound(value / 10000L, 0, 255);
			break;
		}
		case SAVEMENU_MUSIC:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				if (value == 0 || checkMusic(value))
					menu->music = value;
			break;
		}
		case SAVEMENU_MIDI:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
				menu->music = find_or_make_midi_music(convert_from_old_midi_id(vbound(value / 10000, MAXMIDIS-MIDIOFFSET_ZSCRIPT, -4)));
			break;
		}
		
		default: return false;
	}
	return true;
}
std::optional<int32_t> savemenu_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;
	switch (command)
	{
		case SAVEMENU_GETNAME:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto arrayptr = get_register(sarg1);
				if (ArrayH::setArray(arrayptr, menu->name, true) == SH::_Overflow)
					Z_scripterrlog("Array supplied to 'SaveMenu->GetName()' not large enough, and couldn't be resized!\n");
			}
			break;
		}
		case SAVEMENU_SETNAME:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto arrayptr = get_register(sarg1);
				ArrayH::getString(arrayptr, menu->name, SAVEMENU_NAME_LENGTH);
			}
			break;
		}
		case SAVEMENU_REMOVE_OPT:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto opt_idx = get_register(sarg1) / 10000L;
				if (unsigned(opt_idx) < menu->options.size())
				{
					auto it = menu->options.begin();
					std::advance(it, opt_idx);
					menu->options.erase(it);
				}
			}
			break;
		}
		case SAVEMENU_ADD_OPT:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto opt_idx = get_register(sarg1) / 10000L;
				if (unsigned(opt_idx) >= menu->options.size())
					menu->options.emplace_back();
				else
				{
					auto it = menu->options.begin();
					std::advance(it, opt_idx);
					menu->options.insert(it, {});
				}
			}
			break;
		}
		case SAVEMENU_SWAP_OPT:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto opt_idx = get_register(sarg1) / 10000L;
				auto opt_idx2 = get_register(sarg2) / 10000L;
				if (unsigned(opt_idx) >= menu->options.size())
					break;
				if (unsigned(opt_idx2) >= menu->options.size())
					break;
				if (opt_idx == opt_idx2)
					break;
				zc_swap(menu->options[opt_idx], menu->options[opt_idx2]);
			}
			break;
		}
		case SAVEMENU_OPT_GET_STRING:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto opt_idx = get_register(sarg1) / 10000L;
				if (unsigned(opt_idx) < menu->options.size())
				{
					SaveMenuOption& opt = menu->options[opt_idx];
					auto arrayptr = get_register(sarg2);
					if (ArrayH::setArray(arrayptr, opt.text, true) == SH::_Overflow)
						Z_scripterrlog("Array supplied to 'SaveMenu->GetString()' not large enough, and couldn't be resized!\n");
				}
			}
			break;
		}
		case SAVEMENU_OPT_SET_STRING:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto opt_idx = get_register(sarg1) / 10000L;
				if (unsigned(opt_idx) < menu->options.size())
				{
					SaveMenuOption& opt = menu->options[opt_idx];
					auto arrayptr = get_register(sarg2);
					ArrayH::getString(arrayptr, opt.text, SAVEMENU_STRING_LENGTH);
				}
			}
			break;
		}
		case SAVEMENU_OPEN:
		{
			if (SaveMenu* menu = checkSaveMenu(GET_REF(savemenuref)))
			{
				auto cursor = menu->run();
				SET_D(rEXP1, (cursor ? *cursor : -1) * 10000L);
				if (Quit)
					return RUNSCRIPT_STOPPED;
			}
			break;
		}

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}

// savemenu arrays.

static ArrayRegistrar SAVEMENU_FLAGS_registrar(SAVEMENU_FLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<SaveMenu, &SaveMenu::flags, SMENU_FLAG_COUNT> impl;
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SAVEMENU_OPT_FLAGS_registrar(SAVEMENU_OPT_FLAGS, []{
	static ScriptingArray_ObjectComputed<SaveMenu, int> impl(
		[](SaveMenu* menu){
			return menu->options.size();
		},
		[](SaveMenu* menu, int index) -> int {
			return menu->options[index].flags;
		},
		[](SaveMenu* menu, int index, int value){
			menu->options[index].flags = (save_menu_option_flags)value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SAVEMENU_OPT_COLOR_TEXT_registrar(SAVEMENU_OPT_COLOR_TEXT, []{
	static ScriptingArray_ObjectComputed<SaveMenu, int> impl(
		[](SaveMenu* menu){
			return menu->options.size();
		},
		[](SaveMenu* menu, int index) -> int {
			return menu->options[index].color;
		},
		[](SaveMenu* menu, int index, int value){
			menu->options[index].color = value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SAVEMENU_OPT_COLOR_PICKED_registrar(SAVEMENU_OPT_COLOR_PICKED, []{
	static ScriptingArray_ObjectComputed<SaveMenu, int> impl(
		[](SaveMenu* menu){
			return menu->options.size();
		},
		[](SaveMenu* menu, int index) -> int {
			return menu->options[index].picked_color;
		},
		[](SaveMenu* menu, int index, int value){
			menu->options[index].picked_color = value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SAVEMENU_OPT_FONTS_registrar(SAVEMENU_OPT_FONTS, []{
	static ScriptingArray_ObjectComputed<SaveMenu, int> impl(
		[](SaveMenu* menu){
			return menu->options.size();
		},
		[](SaveMenu* menu, int index) -> int {
			return menu->options[index].font;
		},
		[](SaveMenu* menu, int index, int value){
			menu->options[index].font = value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar SAVEMENU_OPT_FRZSCR_registrar(SAVEMENU_OPT_FRZSCR, []{
	static ScriptingArray_ObjectComputed<SaveMenu, int> impl(
		[](SaveMenu* menu){
			return menu->options.size();
		},
		[](SaveMenu* menu, int index) -> int {
			return menu->options[index].gen_script;
		},
		[](SaveMenu* menu, int index, int value){
			menu->options[index].gen_script = value;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());
