#ifndef _ZQ_FILES_H_
#define _ZQ_FILES_H_

#include "base/zc_alleg.h"

extern char qtbuf[31];

int32_t NewQuestFile(int32_t template_slot);
int32_t onNew();
void applyRuleset(int32_t ruleset, byte* qrptr = nullptr);
void applyRuleTemplate(int32_t ruleTemplate, byte* qrptr = nullptr);
bool hasCompatRulesEnabled();
int32_t PickRuleset();
int32_t PickRuleTemplate();
int32_t onSave();
int32_t onSaveAs();
int32_t open_quest(char const* path);
int32_t customOpen(char const* path);
char* get_qst_name(char const* def_path = nullptr);
int32_t onOpen();
int32_t onTileset();
int32_t onRevert();
int32_t get_import_map_bias();
int32_t onImport_Map();
int32_t onExport_Map();
int32_t onImport_DMaps();
int32_t onExport_DMaps();
int32_t onImport_Pals();
int32_t onExport_Pals();
int32_t onImport_Msgs();
int32_t onImport_StringsTSV();
int32_t onExport_Msgs();
int32_t onExport_MsgsText();
int32_t onExport_StringsTSV();
int32_t onImport_Combos();
int32_t onExport_Combos();
int32_t onImport_Tiles();
int32_t onExport_Tiles();
int32_t onImport_ZGP();
int32_t onExport_ZGP();
int32_t onExport_Tilepack();
int32_t onImport_Tilepack();
int32_t onImport_Tilepack_To();

int32_t onExport_Combopack();
int32_t onImport_Combopack_To();
int32_t onImport_Combopack();

int32_t onImport_Comboaliaspack();
int32_t onImport_Comboaliaspack_To();
int32_t onExport_Comboaliaspack();

void center_zq_files_dialogs();

int32_t writezdoorsets(PACKFILE *f, int32_t first, int32_t count);
int32_t readzdoorsets(PACKFILE *f, int32_t first, int32_t count, int32_t deststart);
int32_t writeonezdoorset(PACKFILE *f, int32_t index);
int32_t readonezdoorset(PACKFILE *f, int32_t index);

int32_t onExport_Doorset();
int32_t onImport_Doorset();

#endif
 
