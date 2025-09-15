#include "base/qrs.h"
#include "base/zc_math.h"
#include "zc/render.h"
#include "zc/scripting/arrays.h"

// Input arrays.

static ArrayRegistrar MOUSEARR_registrar(MOUSEARR, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return 6; },
		[](int, int index) {
			switch (index)
			{
				case 0: //MouseX
					return get_mouse_state(0) * 10000;
				case 1: //MouseY
				{
					int32_t mousequakeoffset = 56+((int32_t)(zc::math::Sin((double)(quakeclk*int64_t(2)-frame))*4));
					int32_t tempoffset = (quakeclk > 0) ? mousequakeoffset : (get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
					int32_t topOffset= (112-tempoffset);
					return (get_mouse_state(1)-topOffset) * 10000;
				}
				case 2: //MouseZ
					return get_mouse_state(2) * 10000;
				case 3: //Left Click
					return (get_mouse_state(3) & 0x1) ? 10000 : 0;
				case 4: //Right Click
					return (get_mouse_state(3) & 0x2) ? 10000 : 0;
				case 5: //Middle Click
					return (get_mouse_state(3) & 0x4) ? 10000 : 0;
				default: NOTREACHED();
			}
		},
		[](int ref, int index, int value) {
			switch (index)
			{
				case 0: //MouseX
				{
					auto [x, y] = rti_game.local_to_world(value / 10000, mouse_y);
					position_mouse(x, y);
					break;	
				}
				case 1: //MouseY
				{
					int32_t mousequakeoffset = 56+((int32_t)(zc::math::Sin((double)(quakeclk*int64_t(2)-frame))*4));
					int32_t tempoffset = (quakeclk > 0) ? mousequakeoffset :(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
					auto [x, y] = rti_game.local_to_world(mouse_x, value / 10000 + tempoffset);
					position_mouse(x, y);
					break;
					
				}
				case 2: //MouseZ
				{
					position_mouse_z(value / 10000);
					break;
				}
				case 3: //Left Click
				{
					if ( value ) mouse_b |= 1;
					else mouse_b &= ~1;
					break;
				}
				case 4: //Right Click
				{
					if ( value ) mouse_b |= 2;
					else mouse_b &= ~2;
					break;
				}
				case 5: //Middle Click
				{
					if ( value ) mouse_b |= 4;
					else mouse_b &= ~4;
					break;
				}
				default: NOTREACHED();
			}

			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(false);
	return &impl;
}());

static ArrayRegistrar RAWKEY_registrar(RAWKEY, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return 127; },
		[](int, int index) -> bool {
			return key_current_frame[index];
		},
		[](int, int index, bool value) {
			key_current_frame[index] = value;
			key[index] = value;
			_key[index] = value;
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DISABLEKEY_registrar(DISABLEKEY, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return 127; },
		[](int, int index) -> bool {
			return disabledKeys[index];
		},
		[](int, int index, bool value) {
			switch (index)
			{
				case KEY_F7:
				case KEY_F8:
				case KEY_F9:
					scripting_log_error_with_context("The key {} passed to is system-reserved, and cannot be disabled", index);
					return false;

				default:
					disabledKeys[index] = value;
			}

			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar KEYINPUT_registrar(KEYINPUT, []{
	static ScriptingArray_GlobalCArray impl(KeyInput, comptime_array_size(KeyInput));
	impl.setMul10000(true);
	impl.setSideEffect([](int index){
		switch (index)
		{
			case KEY_F6: onTryQuit(); break;
			case KEY_F3: Paused = !Paused; break;
			case KEY_F4: Paused = true; Advance = true; break;
		}
	});
	return &impl;
}());

static ArrayRegistrar KEYPRESS_registrar(KEYPRESS, []{
	static ScriptingArray_GlobalCArray impl(KeyPress, comptime_array_size(KeyPress));
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar DISABLEBUTTON_registrar(DISABLEBUTTON, []{
	static ScriptingArray_GlobalCArray impl(disable_control, comptime_array_size(disable_control));
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar READKEY_registrar(READKEY, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return 127; },
		[](int, int index) -> bool {
			return zc_readkey(index, true);
		},
		[](int, int, bool) {
			return false;
		}
	);
	impl.setMul10000(true);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar JOYPADPRESS_registrar(JOYPADPRESS, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return MAX_JOYSTICK_BUTTONS; },
		[](int, int index) -> bool {
			return joybtn(index);
		},
		[](int, int, bool) {
			return false;
		}
	);
	impl.setMul10000(true);
	impl.setReadOnly();
	return &impl;
}());

static ArrayRegistrar KEYBINDINGS_registrar(KEYBINDINGS, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return 14; },
		[](int, int index) {
			switch (index) {
				case 0: return DUkey;
				case 1: return DDkey;
				case 2: return DLkey;
				case 3: return DRkey;
				case 4: return Akey;
				case 5: return Bkey;
				case 6: return Skey;
				case 7: return Lkey;
				case 8: return Rkey;
				case 9: return Pkey; /*map*/
				case 10: return Exkey1;
				case 11: return Exkey2;
				case 12: return Exkey3;
				case 13: return Exkey4;
				default: NOTREACHED();
			}
		},
		[](int, int index, int value) {
			switch (index) {
				case 0: DUkey = value; break;
				case 1: DDkey = value; break;
				case 2: DLkey = value; break;
				case 3: DRkey = value; break;
				case 4: Akey = value; break;
				case 5: Bkey = value; break;
				case 6: Skey = value; break;
				case 7: Lkey = value; break;
				case 8: Rkey = value; break;
				case 9: Pkey = value; break;
				case 10: Exkey1 = value; break;
				case 11: Exkey2 = value; break;
				case 12: Exkey3 = value; break;
				case 13: Exkey4 = value; break;
				default: NOTREACHED();
			}

			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar BUTTONHELD_registrar(BUTTONHELD, []{
	static ScriptingArray_GlobalCArray impl(button_hold, comptime_array_size(button_hold));
	impl.compatBoundIndex();
	return &impl;
}());

static ArrayRegistrar BUTTONINPUT_registrar(BUTTONINPUT, []{
	static ScriptingArray_GlobalCArray impl(control_state, comptime_array_size(control_state));
	impl.compatBoundIndex();
	impl.setSideEffect([](int index){
		if (index < 11 && get_qr(qr_FIXDRUNKINPUTS))
			drunk_toggle_state[index] = false;
	});
	return &impl;
}());

static ArrayRegistrar BUTTONPRESS_registrar(BUTTONPRESS, []{
	static ScriptingArray_GlobalCArray impl(button_press, comptime_array_size(button_press));
	impl.compatBoundIndex();
	impl.setSideEffect([](int index) {
		if (index < 11 && get_qr(qr_FIXDRUNKINPUTS)) {
			drunk_toggle_state[index] = false;
		}
	});
	return &impl;
}());
