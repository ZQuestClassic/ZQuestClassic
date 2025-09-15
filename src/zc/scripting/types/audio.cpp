#include "base/qrs.h"
#include "zc/scripting/arrays.h"

// Audio arrays.

static ArrayRegistrar AUDIOVOLUME_registrar(AUDIOVOLUME, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return 4; },
		[](int, int index){
			switch (index)
			{
				case 0: //midi volume
					return FFScript::do_getMIDI_volume();
				case 1: //digi volume
					return FFScript::do_getDIGI_volume();
				case 2: //emh music volume
					return FFScript::do_getMusic_volume();
				case 3: //sfx volume
					return FFScript::do_getSFX_volume();
				default: NOTREACHED();
			}
		},
		[](int, int index, int value){
			if (!get_qr(qr_OLD_SCRIPT_VOLUME))
				return false;

			switch (index)
			{
				case 0: //midi volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME) ) 
					{
						FFCore.usr_midi_volume = FFScript::do_getMIDI_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_MIDI_VOLUME,true);
					}
					FFScript::do_setMIDI_volume(value);
					break;
				}
				case 1: //digi volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME) ) 
					{
						FFCore.usr_digi_volume = FFScript::do_getDIGI_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_DIGI_VOLUME,true);
					}
					FFScript::do_setDIGI_volume(value);
					break;
				}
				case 2: //emh music volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME) ) 
					{
						FFCore.usr_music_volume = FFScript::do_getMusic_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_MUSIC_VOLUME,true);
					}
					FFScript::do_setMusic_volume(value);
					break;
				}
				case 3: //sfx volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME) ) 
					{
						FFCore.usr_sfx_volume = FFScript::do_getSFX_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_SFX_VOLUME,true);
					}
					FFScript::do_setSFX_volume(value);
					break;
				}
				default: NOTREACHED();
			}

			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MUSICUPDATEFLAGS_registrar(MUSICUPDATEFLAGS, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int){ return 2; },
		[](int, int index) -> bool {
			return (FFCore.music_update_flags >> index) & 1;
		},
		[](int, int index, bool value){
			SETFLAG(FFCore.music_update_flags, 1 << index, value);
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());
