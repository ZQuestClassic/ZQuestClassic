#ifndef CORE_QUEST_FORMAT_H_
#define CORE_QUEST_FORMAT_H_

#include <cstdint>

// Quest version information categories. These reflect the version details
// from when the quest was last saved.
enum
{
	vZelda, qQuestNumber, vBuild, vBeta, vHeader, vRules, vStrings, vMisc,
	vTiles, vCombos, vCSets, vMaps, vDMaps, vDoors, vItems, vWeaponSprites,
	vColours, vIcons, vGfxPack, vInitData, vGuys, vMIDIs, vCheats, vSaveformat,
	vComboAliases, vHeroSprites, vSubscreen, vItemDropsets, vFFScript, vSFX, vFavourites,
	qMapCount, vLastCompile, vCompatRule, vCompileSetting,
	versiontypesLAST
};

extern int32_t quest_format[versiontypesLAST];

#endif
