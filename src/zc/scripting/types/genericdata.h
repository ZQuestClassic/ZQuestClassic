#ifndef ZC_SCRIPTING_GENERICDATA_H_
#define ZC_SCRIPTING_GENERICDATA_H_

#include "base/containers.h"
#include "base/ints.h"

#include <cstdint>

enum scr_timing
{
	SCR_TIMING_INIT = -1,
	//0
	SCR_TIMING_START_FRAME, SCR_TIMING_POST_COMBO_ANIM, SCR_TIMING_POST_POLL_INPUT,
	SCR_TIMING_POST_FFCS, SCR_TIMING_POST_GLOBAL_ACTIVE,
	//5
	SCR_TIMING_POST_PLAYER_ACTIVE, SCR_TIMING_POST_DMAPDATA_ACTIVE,
	SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN, SCR_TIMING_POST_COMBOSCRIPT,
	SCR_TIMING_POST_PUSHBLOCK,
	//10
	SCR_TIMING_POST_ITEMSPRITE_SCRIPT, SCR_TIMING_POST_ITEMSPRITE_ANIMATE,
	SCR_TIMING_POST_NPC_ANIMATE, SCR_TIMING_POST_EWPN_ANIMATE, SCR_TIMING_POST_EWPN_SCRIPT,
	//15
	SCR_TIMING_POST_OLD_ITEMDATA_SCRIPT, SCR_TIMING_POST_PLAYER_ANIMATE,
	SCR_TIMING_POST_NEW_ITEMDATA_SCRIPT, SCR_TIMING_POST_CASTING,
	SCR_TIMING_POST_LWPN_ANIMATE,
	//20
	SCR_TIMING_POST_DECOPARTICLE_ANIMATE, SCR_TIMING_POST_COLLISIONS_PALETTECYCLE,
	SCR_TIMING_WAITDRAW, SCR_TIMING_POST_GLOBAL_WAITDRAW, SCR_TIMING_POST_PLAYER_WAITDRAW,
	//25
	SCR_TIMING_POST_DMAPDATA_ACTIVE_WAITDRAW, SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN_WAITDRAW,
	SCR_TIMING_POST_SCREEN_WAITDRAW, SCR_TIMING_POST_FFC_WAITDRAW, SCR_TIMING_POST_COMBO_WAITDRAW,
	//30
	SCR_TIMING_POST_ITEM_WAITDRAW, SCR_TIMING_POST_NPC_WAITDRAW, SCR_TIMING_POST_EWPN_WAITDRAW,
	SCR_TIMING_POST_LWPN_WAITDRAW, SCR_TIMING_POST_ITEMSPRITE_WAITDRAW,
	//35
	SCR_TIMING_PRE_DRAW, SCR_TIMING_POST_DRAW, SCR_TIMING_POST_STRINGS, SCR_TIMING_END_FRAME,
	SCR_NUM_TIMINGS
};

struct user_genscript
{
	//Saved vars
	byte _doscript;
	bounded_map<dword,int32_t> data;
	word exitState;
	word reloadState;
	uint32_t eventstate;
	script_config scrconfig;
	
	//Temp Vars
	bool wait_atleast = true;
	bool waitevent;
	scr_timing waituntil = SCR_TIMING_START_FRAME;
	
	void clear();
	void launch();
	void quit();
	size_t dataSize() const
	{
		return data.size();
	}
	void dataResize(int32_t sz)
	{
		data.resize(vbound(sz, 0, 214748));
	}
	void timeExit(byte exState)
	{
		if(!doscript()) return;
		if(exitState & (1<<exState))
			quit();
		else if(reloadState & (1<<exState))
			launch();
	}
	byte& doscript();
	byte const& doscript() const;
	static user_genscript& get(int indx);
private:
	static user_genscript user_scripts[NUMSCRIPTSGENERIC];
	
	user_genscript() = default;
};

user_genscript* checkGenericScr(int32_t ref);

int32_t genericdata_get_register(int32_t reg);
void genericdata_set_register(int32_t reg, int32_t value);

#endif
