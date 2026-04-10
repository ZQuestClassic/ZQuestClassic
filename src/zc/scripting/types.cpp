#include "zc/scripting/types.h"
#include "components/zasm/defines.h"
#include "components/zasm/table.h"

static bool register_routing_table_init;
EngineSubsystem register_routing_table[MAX_REGISTER_ID + 1];

void initializeRegisterRoutingTable()
{
	if (register_routing_table_init)
		return;

	for (int i = 0; i < MAX_REGISTER_ID; i++)
	{
		switch (i)
		{
			case ITEMCOUNT:
				register_routing_table[i] = EngineSubsystem::Item;
				continue;

			case ACTIVESSSPEED:
			case ALLOCATEBITMAPR:
			case COMBOCDM:
			case COMBODDM:
			case COMBOFDM:
			case COMBOIDM:
			case COMBOSDM:
			case COMBOTDM:
			case CURDMAP:
			case CURDSCR:
			case CURLEVEL:
			case CURMAP:
			case CURSCR:
			case GAME_SAVEMENU_F6:
			case GAME_SAVEMENU_GAMEOVER:
			case GAMECHEAT:
			case GAMECLICKFREEZE:
			case GAMECONTDMAP:
			case GAMECONTSCR:
			case GAMEDEATHS:
			case GAMEENTRDMAP:
			case GAMEENTRSCR:
			case GAMEHASPLAYED:
			case GAMEMAXCHEAT:
			case GAMEMAXMAPS:
			case GAMEMOUSECURSOR:
			case GAMENUMMESSAGES:
			case GAMESTANDALONE:
			case GAMETIME:
			case GAMETIMEVALID:
			case GETMIDI:
			case HERO_SCREEN:
			case NOACTIVESUBSC:
			case SCREENSTATEDD:
			case SKIPCREDITS:
			case SKIPF6:
			case TYPINGMODE:
			case ZELDABETA:
			case ZELDABETATYPE:
			case ZELDABUILD:
			case ZELDAVERSION:
			case ZSCRIPTVERSION:
				register_routing_table[i] = EngineSubsystem::Game;
				continue;
		}

		if (auto ref = get_register_ref_dependency(i))
		{
			switch (*ref)
			{
				case REFDIRECTORY: register_routing_table[i] = EngineSubsystem::Directory; break;
				case REFFILE: register_routing_table[i] = EngineSubsystem::File; break;
				case REFITEM: register_routing_table[i] = EngineSubsystem::Item; break;
				case REFMUSIC: register_routing_table[i] = EngineSubsystem::Music; break;
				case REFNPC: register_routing_table[i] = EngineSubsystem::Npc; break;
				case REFSAVEMENU: register_routing_table[i] = EngineSubsystem::SaveMenu; break;
				case REFSPRITE: register_routing_table[i] = EngineSubsystem::Sprite; break;
				case REFWEBSOCKET: register_routing_table[i] = EngineSubsystem::Websocket; break;
			}
		}
	}

	register_routing_table_init = true;
}
