#include "cheats.h"
#include "zelda.h"
#include "hero.h"
#include "guys.h"
#include "zc_init.h"
#include "init.h"
#include <queue>
#include <tuple>
#include <algorithm>

extern HeroClass Hero;

static std::queue<std::tuple<Cheat, int, int, std::string>> cheats;

static const std::vector<std::string> CheatStrings = {
    "None",
    "Life",
    "Magic",
    "Rupies",
    "Bombs",
    "Arrows",
    "Kill",
    "Fast",
    "Clock",
    "Walls",
    "Freeze",
    "Light",
    "GoTo",
    "IgnoreSideView",
    "MaxLife",
    "MaxMagic",
    "MaxBombs",
    "PlayerData",
    "Last",
};

Cheat cheat_from_string(std::string name)
{
    auto it = std::find(std::begin(CheatStrings), std::end(CheatStrings), name.c_str());
    ASSERT(it != std::end(CheatStrings));
    return (Cheat)(it - std::begin(CheatStrings));
}

std::string cheat_to_string(Cheat cheat)
{
    return CheatStrings[cheat];
}

void cheats_enqueue(Cheat cheat, int arg1, int arg2, std::string arg3)
{
    cheats.push({cheat, arg1, arg2, arg3});
}

static void cheats_execute(Cheat cheat, int arg1, int arg2, std::string arg3)
{
    replay_step_cheat(cheat, arg1, arg2, arg3);

    switch (cheat)
    {
    case Life:
    {
        game->set_life(arg1);
    }
    break;

    case Magic:
    {
        game->set_magic(arg1);
    }
    break;

    case Rupies:
    {
        game->set_drupy(arg1);
    }
    break;

    case Bombs:
    {
        for (int32_t i = 0; i < MAXITEMS; i++)
        {
            if (itemsbuf[i].family == itype_bomb || itemsbuf[i].family == itype_sbomb)
                getitem(i, true);
        }

        game->set_bombs(arg1);
        if (arg2 != -1)
            game->set_sbombs(arg2);
    }
    break;

    case Arrows:
    {
        game->set_arrows(arg1);
    }
    break;

    case Kill:
    {
        for (int32_t i = 0; i < guys.Count(); i++)
        {
            if (!(((enemy *)guys.spr(i))->flags & guy_doesntcount))
                ((enemy *)guys.spr(i))->kickbucket();
        }
    }
    break;

    case Fast:
    {
        gofast = gofast ? false : true;
    }
    break;

    case Clock:
    {
        setClock(!getClock());
        cheat_superman = getClock();
    }
    break;

    case Walls:
    {
        toogam = !toogam;
        if (toogam)
        {
            cheat_superman = true;
            setClock(true);
        }
    }
    break;

    case Freeze:
    {
        if (Hero.getAction() == freeze)
        {
            Hero.unfreeze();
        }
        else
        {
            Hero.Freeze();
        }
    }
    break;

    case Light:
    {
        cheats_execute_light = true;
    }
    break;

    case GoTo:
    {
        cheats_execute_goto = true;
        cheat_goto_dmap = arg1;
        cheat_goto_screen = arg2;
    }
    break;

    case IgnoreSideView:
    {
        ignoreSideview = !ignoreSideview;
    }
    break;

    case MaxLife:
    {
        game->set_maxlife(arg1);
    }
    break;

    case MaxMagic:
    {
        game->set_maxmagic(arg1);
    }
    break;

    case MaxBombs:
    {
        game->set_maxbombs(arg1);
    }
    break;

    case PlayerData:
    {
		zinitdata* base = copyIntoZinit(game);
		zinitdata* new_init = apply_init_data_delta(base, arg3);
		if (new_init)
		{
			resetItems(game, new_init, false);
			ringcolor(false);
			delete new_init;
		}
		delete base;
    }
    break;

    case None:
    case Last:
        break;
    }
}

void cheats_execute_queued()
{
    while (!cheats.empty())
    {
        std::apply(cheats_execute, cheats.front());
        cheats.pop();
    }
}
