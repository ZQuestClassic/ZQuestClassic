#ifndef QUESTREPORT_H
#define QUESTREPORT_H

#include "base/zc_alleg.h"

struct mapscr;

void showQuestReport();

int32_t onIntegrityCheckRooms();
int32_t onIntegrityCheckWarps();
int32_t onIntegrityCheckAll();
int32_t onItemLocationReport();
int32_t onEnemyLocationReport();
int32_t onScriptLocationReport();
int32_t onComboLocationReport();
int32_t onComboTypeLocationReport();
int32_t onBuggedNextComboLocationReport();
void BuggedNextComboLocationReport();
int32_t onWhatWarpsReport();

//Integrity booleans
bool integrityBoolSpecialItem(mapscr* ts);
bool integrityBoolEnemiesSecret(mapscr* ts);
bool integrityBoolEnemiesItem(mapscr* ts);
bool integrityBoolStringNoGuy(mapscr* ts);
bool integrityBoolGuyNoString(mapscr* ts);
bool integrityBoolRoomNoGuy(mapscr* ts);
bool integrityBoolRoomNoString(mapscr* ts);
bool integrityBoolRoomNoGuyNoString(mapscr* ts);
bool integrityBoolSaveCombo(mapscr* ts, int32_t ctype);
bool integrityBoolUnderCombo(mapscr* ts, int32_t ctype);

#endif
