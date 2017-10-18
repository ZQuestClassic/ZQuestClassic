#include <angelscript.h>
#include "util.h"
#include "aszc.h"
#include <decorations.h>
#include <guys.h>
#include <link.h>
#include <maps.h>
#include <sound.h>
#include <sprite.h>
#include <tiles.h>
#include <zc_alleg.h>
#include <zc_sys.h>
#include <zelda.h>

void asSFX(int sound, int x)
{
    sfx(sound, pan(x));
}

void asStopSFX(int sound)
{
    stop_sfx(sound);
}

bool asQuestRuleEnabled(int rule)
{
    return get_bit(quest_rules, rule);
}

int getSpriteTile(int spr)
{
    return wpnsbuf[spr].tile;
}

bool asIsWater(int pos)
{
    return iswater(tmpscr->data[pos]);
}

// guygrid[] tracks how long it's been since a combo spawned an enemy, I think
void asSetGuyGridAt(int x, int y, int value)
{
    guygrid[(y&0xF0)+(x>>4)]=value;
}

void asChangeCounter(int id, short value)
{
    game->change_counter(value, id);
}

void asChangeDCounter(int id, short value)
{
    game->change_dcounter(value, id);
}

int asGetGeneric(int id)
{
    return game->get_generic(id);
}

void asSetLevelItem(int level, int item)
{
    game->lvlitems[level]|=item;
}

int asGetGravity()
{
    return zinit.gravity;
}

int asGetTerminalVelocity()
{
    return zinit.terminalv;
}

void asDecGuyCount(int screen)
{
    game->guys[screen]-=1;
}


mapscr* asGetTmpscr(int idx)
{
    return &tmpscr[idx];
}

mapscr* asGetTmpscr2(int idx)
{
    return &tmpscr2[idx];
}

mapscr* asGetTmpscr3(int idx)
{
    return &tmpscr3[idx];
}


enum
{
    WPNPROP_TILE, WPNPROP_CSETS, WPNPROP_FRAMES
};

int asGetWeaponProp(int id, int prop)
{
    wpndata& data=wpnsbuf[id];
    
    switch(prop)
    {
    case WPNPROP_TILE:
        return data.tile;
    case WPNPROP_CSETS:
        return data.csets;
    case WPNPROP_FRAMES:
        return data.frames;
    default:
        return 0;
    }
}

enum
{
    COMBOPROP_TYPE
};

int asGetComboProp(int id, int prop)
{
    newcombo& combo=combobuf[id];
    
    switch(prop)
    {
    case COMBOPROP_TYPE:
        return combo.type;
    default:
        return 0;
    }
}

enum
{
    CCPROP_BLOCK_ENEMIES
};

int asGetComboClassProp(int type, int prop)
{
    comboclass& cc=combo_class_buf[type];
    
    switch(prop)
    {
    case CCPROP_BLOCK_ENEMIES:
        return cc.block_enemies;
    default:
        return 0;
    }
}

enum
{
    ITEMPROP_FAMILY, ITEMPROP_LEVEL, ITEMPROP_POWER, ITEMPROP_MAGIC,
    ITEMPROP_TILE, ITEMPROP_CSETS, ITEMPROP_SOUND, ITEMPROP_FLAGS,
    ITEMPROP_MISC1, ITEMPROP_WPN2, ITEMPROP_PICKUP_SCRIPT
};

int asGetItemProp(int id, int prop)
{
    itemdata& data=itemsbuf[id];
    
    switch(prop)
    {
    case ITEMPROP_FAMILY:
        return data.family;
    case ITEMPROP_LEVEL:
        return data.fam_type;
    case ITEMPROP_POWER:
        return data.power;
    case ITEMPROP_MAGIC:
        return data.magic;
    case ITEMPROP_TILE:
        return data.tile;
    case ITEMPROP_CSETS:
        return data.csets;
    case ITEMPROP_SOUND:
        return data.usesound;
    case ITEMPROP_FLAGS:
        return data.flags;
    case ITEMPROP_MISC1:
        return data.misc1;
    case ITEMPROP_WPN2:
        return data.wpn2;
    case ITEMPROP_PICKUP_SCRIPT:
        return data.collect_script;
    default:
        return 0;
    }
}

void asSetItem(int id, bool value)
{
    game->set_item(id, value);
}



asIScriptObject* asGetGuyObject(int index)
{
    asIScriptObject* obj=guys.spr(index)->getScriptObject();
    obj->AddRef();
    return obj;
}

int asNumGuys()
{
    return guys.Count();
}

#define GUYPROP_FAMILY  0
#define GUYPROP_HP      1
#define GUYPROP_STEP    2
#define GUYPROP_MISC2   3
#define GUYPROP_MISC9   4
#define GUYPROP_MISC10  5
#define GUYPROP_FLAGS2  6
#define GUYPROP_BOSSPAL 7

int asGetGuyProp(int id, int prop)
{
    const guydata& gd=guysbuf[id&0xFFF];
    
    switch(prop)
    {
    case GUYPROP_FAMILY:
        return gd.family;
    case GUYPROP_HP:
        return gd.hp;
    case GUYPROP_STEP:
        return gd.step;
    case GUYPROP_MISC2:
        return gd.misc2;
    case GUYPROP_MISC9:
        return gd.misc9;
    case GUYPROP_MISC10:
        return gd.misc10;
    case GUYPROP_FLAGS2:
        return gd.flags2;
    case GUYPROP_BOSSPAL:
        return gd.bosspal;
    default:
        return 0;
    }
}

void asSwapGuys(int index1, int index2)
{
    guys.swap(index1, index2);
}


asIScriptObject* asGetItemObject(int index)
{
    asIScriptObject* obj=items.spr(index)->getScriptObject();
    obj->AddRef();
    return obj;
}

int asNumItems()
{
    return items.Count();
}

void asDeleteItem(int index)
{
    items.del(index);
}


asIScriptObject* asGetLwpnObject(int index)
{
    asIScriptObject* obj=Lwpns.spr(index)->getScriptObject();
    obj->AddRef();
    return obj;
}

int asNumLwpns()
{
    return Lwpns.Count();
}


asIScriptObject* asGetEwpnObject(int index)
{
    asIScriptObject* obj=Ewpns.spr(index)->getScriptObject();
    obj->AddRef();
    return obj;
}

int asNumEwpns()
{
    return Ewpns.Count();
}

void asDeleteEwpn(int index)
{
    Ewpns.del(index);
}


void asPlaceItem(int x, int y, int id, int pickupFlags, int clk)
{
    // Not the same as additem() - doesn't check qr_NOITEMOFFSET
    items.add(new item(fix(x), fix(y), fix(0), id, pickupFlags, clk));
}


void asAddDecoration(int type, int x, int y)
{
    switch(type)
    {
    case  dBUSHLEAVES:
        decorations.add(new dBushLeaves(fix(x), fix(y), dBUSHLEAVES, 0));
        break;
        
    case dFLOWERCLIPPINGS:
        decorations.add(new dFlowerClippings(fix(x), fix(y), dFLOWERCLIPPINGS, 0));
        break;
        
    case dGRASSCLIPPINGS:
        decorations.add(new dGrassClippings(fix(x), fix(y), dGRASSCLIPPINGS, 0));
        break;
        
    case dHAMMERSMACK:
        decorations.add(new dHammerSmack(fix(x), fix(y), dHAMMERSMACK, 0));
        break;
    }
}


void asMoveChainLinks(int dx, int dy)
{
    for(int i=0; i<chainlinks.Count(); i++)
    {
        chainlinks.spr(i)->x+=dx;
        chainlinks.spr(i)->y+=dy;
    }
}

bool asInputUp()
{
    return control_state[0];
}

bool asInputDown()
{
    return control_state[1];
}

bool asInputLeft()
{
    return control_state[2];
}

bool asInputRight()
{
    return control_state[3];
}

bool asCAbtn()
{
    return control_state[4];
}

bool asCBbtn()
{
    return control_state[5];
}

bool asCSbtn()
{
    return control_state[6];
}

bool asCMbtn()
{
    return key[KEY_ESC]||joybtn(Mbtn);
}

bool asCLbtn()
{
    return control_state[7];
}

bool asCRbtn()
{
    return control_state[8];
}

bool asCPbtn()
{
    return control_state[9];
}

bool asCEx1btn()
{
    return control_state[10];
}

bool asCEx2btn()
{
    return control_state[11];
}

bool asCEx3btn()
{
    return control_state[12];
}

bool asCEx4btn()
{
    return control_state[13];
}

bool asInputAxisUp()
{
    return control_state[14];
}

bool asInputAxisDown()
{
    return control_state[15];
}

bool asInputAxisLeft()
{
    return control_state[16];
}

bool asInputAxisRight()
{
    return control_state[17];
}

bool asRUp()
{
    return rButton(Up,Udown);
}

bool asRDown()
{
    return rButton(Down,Ddown);
}

bool asRLeft()
{
    return rButton(Left,Ldown);
}

bool asRRight()
{
    return rButton(Right,Rdown);
}

bool asRAbtn()
{
    return rButton(cAbtn,Adown);
}

bool asRBbtn()
{
    return rButton(cBbtn,Bdown);
}

bool asRSbtn()
{
    return rButton(cSbtn,Sdown);
}

bool asRMbtn()
{
    return rButton(cMbtn,Mdown);
}

bool asRLbtn()
{
    return rButton(cLbtn,LBdown);
}

bool asRRbtn()
{
    return rButton(cRbtn,RBdown);
}

bool asRPbtn()
{
    return rButton(cPbtn,Pdown);
}

bool asREx1btn()
{
    return rButton(cEx1btn,Ex1down);
}

bool asREx2btn()
{
    return rButton(cEx2btn,Ex2down);
}

bool asREx3btn()
{
    return rButton(cEx3btn,Ex3down);
}

bool asREx4btn()
{
    return rButton(cEx4btn,Ex4down);
}

bool asRAxisUp()
{
    return rButton(AxisUp,AUdown);
}

bool asRAxisDown()
{
    return rButton(AxisDown,ADdown);
}

bool asRAxisLeft()
{
    return rButton(AxisLeft,ALdown);
}

bool asRAxisRight()
{
    return rButton(AxisRight,ARdown);
}
