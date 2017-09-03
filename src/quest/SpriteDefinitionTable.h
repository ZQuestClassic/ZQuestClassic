#ifndef WEAPONDEFINITIONTABLE_H
#define WEAPONDEFINITIONTABLE_H

#include <string>
#include "../zdefs.h"
#include "QuestRefs.h"

class SpriteDefinitionTable;
class Quest;

struct wpndata
{
    wpndata()
    {
        clear();
    }

    void clear()
    {
        name = std::string("");
        tile = 0;
        misc = 0;
        csets = 0;
        frames = 0;
        speed = 0;
        type = 0;
        script = 0;
    }

    const static int WF_BEHIND = 0x10;	//Weapon renders behind other sprites

    std::string name;
    word tile;
    byte misc;                                                // 000bvhff (vh:flipping, f:flash (1:NES, 2:BSZ))
    byte csets;                                               // ffffcccc (f:flash cset, c:cset)
    byte frames;                                              // animation frame count
    byte speed;                                               // animation speed
    byte type;                                                // used by certain weapons
    word script;
};

/*
 * Stores which sprites in the sprite definition table to use for "hard-coded"
 * parts of the ZC game engine.
 * Presumably in the long term, all of these will become editable in ZQuest,
 * at which point either this data structure will become editable via the GUI,
 * by scripts, or be obsoleted entirely.
 * For now, this index is a way to check, at the time a quest is loaded (and
 * not in the middle of editing/playing a quest ;) ) that all sprites needed
 * during play/editing are indeed present in sprite table.
 */
struct SpecialSpriteIndex
{
public:
    SpecialSpriteIndex();

    /*
     * Checks that all entries in the index point to valid weapons in the given
     * weapons definition table.
     */
    bool checkConsistency(Quest &quest);

    SpriteDefinitionRef messageMoreIndicator;       // sprite used for the "more" indicator in message strings
    SpriteDefinitionRef bushLeavesDecoration;       // animated combos
    SpriteDefinitionRef flowerClippingsDecoration;
    SpriteDefinitionRef grassClippingsDecoration;
    SpriteDefinitionRef tallGrassDecoration;
    SpriteDefinitionRef ripplesDecoration;
    SpriteDefinitionRef nayruShieldFront;           
    SpriteDefinitionRef nayruShieldBack;
    SpriteDefinitionRef lifeMeterHearts;            // quarter-heart sprites used in the life meter
    SpriteDefinitionRef enemySpawnCloud;            // poof when enemies spawn
    SpriteDefinitionRef enemyDeathCloud;            // poof when enemies die
    SpriteDefinitionRef smallShadow;                // shadow sprite of small (1-tile) enemies
    SpriteDefinitionRef largeShadow;                // shadow sprite of large (4-tile) enemies
    SpriteDefinitionRef linkSwim;               
    SpriteDefinitionRef linkSlash;
    SpriteDefinitionRef magicMeter;                 // unit of the magic meter on the subscreen
    SpriteDefinitionRef flickeringFlame;            // enemy flame sprite (special in that it flickers over time)
    SpriteDefinitionRef flickeringFlame2;
    SpriteDefinitionRef flickeringFireTrail;        // enemy fire trail (special in that it flickers over time)
    SpriteDefinitionRef subscreenVine;
    SpriteDefinitionRef npcTemplate;                // start of NPC tiles
    SpriteDefinitionRef defaultLinkWeaponSprite;    // used if a weapon item for some reason does not have a wpn set
    SpriteDefinitionRef defaultFireSprite;          // for some reason needed in conjunction with the warp whistle wind? I don't really understand
    SpriteDefinitionRef defaultBoomerangSprites[3];
    SpriteDefinitionRef defaultBombExplosion;
    SpriteDefinitionRef defaultSuperBombExplosion;
    SpriteDefinitionRef silverSparkle;
    SpriteDefinitionRef fireSparkle;
    SpriteDefinitionRef dinsRocketTrail;
    SpriteDefinitionRef dinsRocketTrailReturn;
    SpriteDefinitionRef nayruRocketTrail;
    SpriteDefinitionRef nayruRocketTrailReturn;
    SpriteDefinitionRef nayruRocketTrail2;
    SpriteDefinitionRef nayruRocketTrailReturn2;
    SpriteDefinitionRef defaultEnemySwordBeamSprite;
    SpriteDefinitionRef defaultEnemyBomb;           // default enemy attack sprites
    SpriteDefinitionRef defaultEnemySuperBomb;
    SpriteDefinitionRef defaultEnemyFireball;
    SpriteDefinitionRef defaultEnemyRock;
    SpriteDefinitionRef defaultEnemyArrow;
    SpriteDefinitionRef defaultEnemyMagic;
    SpriteDefinitionRef defaultEnemyWind;
    SpriteDefinitionRef defaultEnemyBombExplosion;
    SpriteDefinitionRef defaultEnemySuperBombExplosion;
};


/*
 * Stores all sprite definitions in the quest. These are the properties stored
 * in the .qst file (*not* temporary instances of weapon sprites).
 * Each sprite has a wpndata structure associated to it, which contains all of 
 * the sprite data including the name.
 */
class SpriteDefinitionTable
{
public:
    /*
     * Removes all sprite definitions.
     */
    void clear();

    /*
     * Retrives the name of the default sprite located in the given slot. This
     * name only really makes sense for sprites in the CORE module in a blank
     * (empty) quest that hasn't been edited. Returns a 
     * programatically-generated name if no descriptive name exists for the
     * given slot.
     */
    static std::string defaultSpriteName(int slot);

    /*
     * Retrives the sprite definition with given index from the table. The
     * given index must be valid.
     */
    wpndata &getSpriteDefinition(int idx) { return spriteData_[idx]; }

    /*
     * Adds a new sprite at the end of the sprite definition table, with given
     * sprite data.
     */
    void addSpriteDefinition(const wpndata &data);

    /*
     * The number of sprites currently in the sprite definition table.
     */
    uint32_t getNumSpriteDefinitions() const { return (int)spriteData_.size(); }

    /*
     * Returns whether the given sprite index is valid (in the table).
     */
    bool isValid(int slot) { return slot >= 0 && slot < (int)spriteData_.size(); }

private:

    std::vector<wpndata> spriteData_;
};


#endif
