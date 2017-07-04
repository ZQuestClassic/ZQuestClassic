#ifndef WEAPONDEFINITIONTABLE_H
#define WEAPONDEFINITIONTABLE_H

#include <string>
#include "../zdefs.h"
#include "QuestRefs.h"

class SpriteDefinitionTable;
class Quest;

struct wpndata
{
    const static int WF_BEHIND = 0x10;	//Weapon renders behind other sprites

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
* Each sprite has a name and an itemdata structure associated to it, the latter
* of which contains all of the non-name data.
*/
class SpriteDefinitionTable
{
public:
    /*
     * Removes all item definitions.
     */
    void clear();

    /*
     * Computes the default sprite name for the sprite with given slot ID in the
     * sprite definition table.
     * These names are based on the order sprites were hard-coded in very old
     * versions of ZC. Sprite with slots that don't correspond to legacy 
     * sprites get a procedurally-generated name.
     */
    static std::string defaultSpriteName(int slot);

    /*
     * Retrives the sprite definition with given index from the table. The
     * given index must be valid.
     */
    wpndata &getSpriteDefinition(int idx) { return spriteData_[idx]; }

    /*
     * Retrives the name of the sprite with given index in the sprite
     * definition table. Crashes if the index is invalid.
     */
    const std::string &getSpriteName(int idx) { return spriteNames_[idx]; }

    /*
     * Adds a new sprite at the end of the sprite definition table, with given
     * settings and name.
     */
    void addSpriteDefinition(const wpndata &data, const std::string &name);

    /*
     * Overwrites the name of the sprite in the given slot in the definition
     * table with a new name. The index must be valid (crashes otherwise).
     */
    void setSpriteName(int idx, const std::string &name);

    /*
     * The number of sprites currently in the weapon definition table.
     */
    uint32_t getNumSpriteDefinitions() const { return (int)spriteData_.size(); }

    /*
     * Returns whether the given sprite index is valid (in the table).
     */
    bool isValid(int slot) { return slot >= 0 && slot < (int)spriteData_.size(); }

private:

    std::vector<wpndata> spriteData_;
    std::vector<std::string> spriteNames_;

};


#endif
