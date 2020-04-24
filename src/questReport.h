#ifndef QUESTREPORT_H
#define QUESTREPORT_H

#include "zc_alleg.h"

struct mapscr;

void enlargeIntegrityReportDialog();

void showQuestReport(int bg,int fg);

int onIntegrityCheckRooms();
int onIntegrityCheckWarps();
int onIntegrityCheckAll();
int onItemLocationReport();
int onEnemyLocationReport();
int onScriptLocationReport();
int onComboLocationReport();
int onComboTypeLocationReport();
int onBuggedNextComboLocationReport();
void BuggedNextComboLocationReport();
int onWhatWarpsReport();

//Integrity booleans
bool integrityBoolSpecialItem(mapscr* ts);
bool integrityBoolEnemiesSecret(mapscr* ts);
bool integrityBoolEnemiesItem(mapscr* ts);
bool integrityBoolStringNoGuy(mapscr* ts);
bool integrityBoolGuyNoString(mapscr* ts);
bool integrityBoolRoomNoGuy(mapscr* ts);
bool integrityBoolRoomNoString(mapscr* ts);
bool integrityBoolRoomNoGuyNoString(mapscr* ts);
bool integrityBoolSaveCombo(mapscr* ts, int ctype);
bool integrityBoolUnderCombo(mapscr* ts, int ctype);

#endif
