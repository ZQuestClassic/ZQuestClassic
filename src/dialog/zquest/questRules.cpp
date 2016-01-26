#include "questRules.h"
#include "../../gui/factory.h"
#include "../../zdefs.h"
#include "../../zsys.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <string>

extern zinitdata zinit;
extern byte quest_rules[QUESTRULES_SIZE];

using namespace GUI;

namespace
{

struct RuleData
{
    int id;
    const char* description;
};

const RuleData animationRules[]={
  { qr_BSZELDA, "BS-Zelda Animation" },
  { qr_COOLSCROLL, "Circle Opening/Closing Wipes" },
  { qr_OVALWIPE, "Oval Opening/Closing Wipes" },
  { qr_TRIANGLEWIPE, "Triangle Opening/Closing Wipes" },
  { qr_SMASWIPE, "Super Mario All-Stars Opening/Closing Wipes" },
  { qr_EXPANDEDLTM, "Expanded Link Tile Modifiers" },
  { qr_NOSCROLL, "No Screen Scrolling" },
  { qr_FASTDNGN, "Fast Dungeon Scrolling" },
  { qr_VERYFASTSCROLLING, "Very Fast Scrolling" },
  { qr_SMOOTHVERTICALSCROLLING, "Smooth Vertical Scrolling" },
  { qr_FFCSCROLL, "FFCs Are Visible While The Screen Is Scrolling" },
  { qr_FADE, "Interpolated Fading" },
  { qr_FADECS5, "Fade CSet 5" },
  { qr_LINKFLICKER, "Invincible Link Flickers" },
  { qr_HOLDITEMANIMATION, "Items Held Above Link's Head Continue To Animate" },
  { qr_NOBOMBPALFLASH, "Bomb Explosions Don't Flash Palette" },
  { qr_LAYER12UNDERCAVE, "Layers 1 and 2 Are Drawn Under Caves" },
  { -1, "" }
};

const RuleData comboRules[]={
  { qr_DROWN, "Link Drowns in Walkable Water" },
  { qr_SMARTSCREENSCROLL, "Smart Screen Scrolling" },
  { qr_SOLIDBLK, "Can't Push Blocks Onto Unwalkable Combos" },
  { qr_HESITANTPUSHBLOCKS, "Push Blocks Don't Move When Bumped" },
  { qr_INSTABURNFLAGS, "Burn Flags Are Triggered Instantly" },
  { qr_SWORDMIRROR, "Magic Mirror/Prism Combos Reflect Sword Beams" },
  { qr_WHIRLWINDMIRROR, "Magic Mirrors Reflect Whistle Whirlwinds" },
  { qr_CMBCYCLELAYERS, "Combo Cycling On Layers" },
  { qr_DMGCOMBOPRI, "Full Priority Damage Combos" },
  { qr_WARPSIGNOREARRIVALPOINT, "Warps Ignore Arrival X/Y Position When Setting Continue Screen" },
  { qr_NOARRIVALPOINT, "Use Warp Return Points Only" },
  { qr_NOSCROLLCONTINUE, "Scrolling Warps Don't Set The Continue Point" },
  { qr_OLDSTYLEWARP, "Use Old-Style Warp Detection (NES Movement Only)" },
  { qr_DMGCOMBOLAYERFIX, "Damage Combos Work On Layers 1 And 2" },
  { qr_HOOKSHOTLAYERFIX, "Hookshot Grab Combos Work On Layers 1 And 2" },
  { -1, "" }
};

const RuleData itemRules[]={
  { qr_ENABLEMAGIC, "Enable Magic" },
  { qr_Z3BRANG_HSHOT, "New Boomerang/Hookshot" },
  { qr_TRUEARROWS, "True Arrows" },
  { qr_NODIVING, "No Diving" },
  { qr_TEMPCANDLELIGHT, "Dark Rooms Stay Lit Only While Fire Is On Screen" },
  { qr_FIREPROOFLINK, "Link Isn't Hurt By Own Fire Weapons" },
  { qr_OUCHBOMBS, "Link's Bombs Hurt Link" },
  { qr_RINGAFFECTDAMAGE, "Rings Affect Damage Combos" },
  { qr_QUICKSWORD, "Quick Sword" },
  { qr_SLASHFLIPFIX, "Flip Right-Facing Slash" },
  { qr_NOWANDMELEE, "Wand Can't Be Used As Melee Weapon" },
  { qr_NOITEMMELEE, "Melee Weapons Can't Pick Up Items" },
  { qr_BRANGPICKUP, "Boomerang/Hookshot Grabs All Items" },
  { qr_HEARTSREQUIREDFIX, "'Hearts Required' Affects Non-Special Items" },
  { qr_4TRI, "Big Triforce Pieces" },
  { qr_3TRI, "3 or 6 Triforce Total" },
  { qr_SLOWCHARGINGWALK, "Slow Walk While Charging" },
  { qr_LENSHINTS, "Show Secret Combo Hints When Using The Lens" },
  { qr_RAFTLENS, "Lens Shows Raft Paths" },
  { qr_LENSSEESENEMIES, "Lens Reveals Invisible Enemies" },
  { qr_NONBUBBLEMEDICINE, "Medicine Doesn't Remove Sword Jinxes" },
  { qr_NONBUBBLEFAIRIES, "Fairies Don't Remove Sword Jinxes" },
  { qr_NONBUBBLETRIFORCE, "Triforce Doesn't Remove Sword Jinxes" },
  { qr_ITEMBUBBLE, "Things That Remove Sword Jinxes Also Remove Item Jinxes" },
  { -1, "" }
};

const RuleData enemyRules[]={
  { qr_NEWENEMYTILES, "Use New Enemy Tiles" },
  { qr_ENEMIESZAXIS, "Enemies Jump/Fly Through Z-Axis" },
  { qr_HIDECARRIEDITEMS, "Hide Enemy-Carried Items" },
  { qr_ALWAYSRET, "Enemies Always Return" },
  { qr_NOTMPNORET, "Temporary No Return Disabled" },
  { qr_KILLALL, "Must Kill All Bosses To Set 'No Return' Screen State" },
  { qr_MEANTRAPS, "Multi-Directional Traps" },
  { qr_MEANPLACEDTRAPS, "Line-of-Sight Traps Can Move Across Entire Screen" },
  { qr_PHANTOMPLACEDTRAPS, "Constant Traps Can Pass Through Enemies" },
  { qr_WALLFLIERS, "Flying Enemies Can Appear on Unwalkable Combos" },
  { qr_BRKNSHLDTILES, "Broken Enemy Shield Tiles" },
  { qr_NOFLASHDEATH, "Enemies Don't Flash When Dying" },
  { qr_SHADOWS, "Show Shadows" },
  { qr_TRANSSHADOWS, "Translucent Shadows" },
  { qr_SHADOWSFLICKER, "Shadows Flicker" },
  { qr_ENEMIESFLICKER, "Enemies Flicker When Hit" },
  { -1, "" }
};

const RuleData nesFixesRules[]={
  { qr_FREEFORM, "Freeform Dungeons" },
  { qr_SAFEENEMYFADE, "Can Safely Trigger Armos/Grave From The South" },
  { qr_ITEMSONEDGES, "Can Use Items/Weapons on Edge of Screen" },
  { qr_LINKDUNGEONPOSFIX, "Fix Link's Position in Dungeons" },
  { qr_RLFIX, "Raft/Ladder Sprite Direction Fix" },
  { qr_NOLEVEL3FIX, "No Palette 3 CSet 6 Fix" },
  { qr_BOMBHOLDFIX, "Link Holds Special Bombs Over His Head" },
  { qr_HOLDNOSTOPMUSIC, "Holding Up Items Doesn't Restart Music" },
  { qr_CAVEEXITNOSTOPMUSIC, "Leaving Item Cellar/Passageway Doesn't Restart Music" },
  { qr_OVERWORLDTUNIC, "Tunic Color Can Change On Overworld" },
  { qr_SWORDWANDFLIPFIX, "Sword/Wand Flip Fix" },
  { qr_PUSHBLOCKCSETFIX, "Push Block CSet Fix" },
  { qr_TRAPPOSFIX, "Trap Position Fix" },
  { qr_NOBORDER, "No Invisible Border on Non-Dungeon Dmaps" },
  { qr_OLDPICKUP, "Items Disappear During Hold-Up" },
  { qr_SUBSCREENOVERSPRITES, "Subscreen Appears Above Sprites" },
  { qr_BOMBDARKNUTFIX, "Correct Bomb/Darknut Interaction" },
  { qr_OFFSETEWPNCOLLISIONFIX, "Correct Offset Enemy Weapon Collision Detection" },
  { qr_ITEMSINPASSAGEWAYS, "Special Items Don't Appear In Passageways" },
  { qr_NOFLICKER, "No NES Sprite Flicker" },
  { qr_FIREPROOFLINK2, "Invincible Link Isn't Hurt By Own Fire Weapons" },
  { qr_NOITEMOFFSET, "No Position Offset Of Screen Items" },
  { qr_LADDERANYWHERE, "Allow Ladder Anywhere" },
  { -1, "" }
};

const RuleData miscRules[]={
  { qr_SELECTAWPN, "Can Select A-Button Weapon On Subscreen" },
  { qr_MORESOUNDS, "More Sound Effects" },
  { qr_FASTFILL, "Fast Heart Refill" },
  { qr_QUARTERHEART, "Enable 1/4 Heart Life Bar" },
  { qr_ALLOWFASTMSG, "Messages Can Be Sped Up With The A Button" },
  { qr_ALLOWMSGBYPASS, "Messages Can Be Bypassed With The B Button" },
  { qr_MSGFREEZE, "Messages Freeze All Action" },
  { qr_MSGDISAPPEAR, "Messages Disappear" },
  { qr_TIME, "Show Time On Subscreen" },
  { qr_HEARTRINGFIX, "Healing Fairy Heart Circle Is Centered Around Link" },
  { qr_NOHEARTRING, "No Healing Fairy Heart Circle" },
  { qr_NOSAVE, "No Saving When Continuing" },
  { qr_NOCONTINUE, "Skip Continue Screen" },
  { qr_NOGUYFIRES, "No Fires In Special Rooms" },
  { qr_NOGUYPOOF, "Special Room Guys Don't Create A Puff When Appearing" },
  { qr_LOG, "Log Game Events To Allegro.log" },
  { qr_SCRIPTERRLOG, "Log Script Errors To Allegro.log" },
  { qr_SHOPCHEAT, "Draining Rupees Can Still Be Spent" },
  { -1, "" }
};

const RuleData compatRules[]={
  { qr_GOTOLESSNOTEQUAL, "Old GOTOLESS Behavior" },
  { qr_OLDLENSORDER, "Old Lens Drawing Order" },
  { qr_NOFAIRYGUYFIRES, "No Fairy Guy Fires" },
  { qr_TRIGGERSREPEAT, "Continuous Step Triggers" },
  { qr_HOOKSHOTDOWNBUG, "Downward Hookshot Bug" },
  { qr_REPLACEOPENDOORS, "Fix Open Door Solidity" },
  { qr_NOSOLIDDAMAGECOMBOS, "No Solid Damage Combos" },
  { qr_OLDHOOKSHOTGRAB, "Old Hookshot Grab Checking" },
  { qr_PEAHATCLOCKVULN, "Peahats Are Vulnerable When Frozen By Clocks" },
  { qr_OFFSCREENWEAPONS, "Weapons With No Collision Detection Move Offscreen" },
  { -1, "" }
};

void addTab(const std::string& name, const RuleData* rules,
  const WidgetFactory& f, TabPanel* tp, std::vector<Checkbox*>& checkboxes)
{
    SerialContainer* column=f.column();
    
    for(int i=0; rules[i].id>-1; i++)
    {
        Checkbox* cb=f.checkbox(rules[i].description);
        cb->setValue(get_bit(quest_rules, rules[i].id)!=0);
        cb->setUserID(rules[i].id);
        column->add(cb);
        checkboxes.push_back(cb);
    }
    
    tp->addTab(name, column);
}

} // namespace

#define CB(func) boost::bind(&QuestRulesEditor::func, this)

Widget* QuestRulesEditor::createDialog(const WidgetFactory& f)
{
    TabPanel* tabPanel;
    
    Window* win=f.window("Quest rules",
      f.column(Contents(
        tabPanel=f.tabPanel(),
        f.buttonRow(Contents(
          f.button("O&K", CB(onOK)),
          f.button("&Cancel", CB(shutDown))
        ))
      ))
    );
    
    checkboxes.reserve(160);
    addTab("&Animation", animationRules, f, tabPanel, checkboxes);
    addTab("C&ombos", comboRules, f, tabPanel, checkboxes);
    addTab("&Items", itemRules, f, tabPanel, checkboxes);
    addTab("&Enemy", enemyRules, f, tabPanel, checkboxes);
    addTab("&NES fixes", nesFixesRules, f, tabPanel, checkboxes);
    addTab("&Misc.", miscRules, f, tabPanel, checkboxes);
    addTab("&Backward compatibility", compatRules, f, tabPanel, checkboxes);
    
    return win;
}

void QuestRulesEditor::onOK()
{
    for(int i=0; i<checkboxes.size(); i++)
    {
        Checkbox* cb=checkboxes[i];
        set_bit(quest_rules, cb->getUserID(), cb->getValue());
    }
    
    shutDown();
    
    // For 2.50.0 and 2.50.1. This can be removed when the quest format changes
    // (i.e. when those versions can't open new quest files).
    if(get_bit(quest_rules, qr_VERYFASTSCROLLING))
        set_bit(quest_rules, qr_FASTDNGN, 1);
    
    // Left over from zq_rules.cpp:
    //this is only here until the subscreen style is selectable by itself
    zinit.subscreen_style=get_bit(quest_rules,qr_COOLSCROLL)?1:0;
}
