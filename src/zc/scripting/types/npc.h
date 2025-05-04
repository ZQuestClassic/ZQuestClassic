#ifndef ZC_SCRIPTING_NPC_H_
#define ZC_SCRIPTING_NPC_H_

#include "base/ints.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include <optional>

class GuyH : public SH
{

public:
	static int32_t loadNPC(const int32_t eid, const char* what);
	
	static enemy *getNPC();
	
	static int32_t getNPCIndex(const int32_t eid);
	
	static int32_t getNPCDMisc(const byte a);
	
	static bool hasHero();
	
	static int32_t getMFlags();
	
	static void clearTemp();
	
private:

	static enemy *tempenemy;
};

std::optional<int32_t> npc_get_register(int32_t reg);
bool npc_set_register(int32_t reg, int32_t value);
std::optional<int32_t> npc_run_command(word command);

#endif
