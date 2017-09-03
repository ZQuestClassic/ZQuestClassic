#ifndef SELECTORS_H
#define SELECTORS_H

#include "quest/ItemDefinitionTable.h"
#include "quest/SpriteDefinitionTable.h"

std::string select_module(const char *prompt, const std::string &selectedModule, bool is_editor, int &exit_status);
ItemDefinitionRef select_item(const char *prompt, const ItemDefinitionRef &item, bool is_editor, int &exit_status);
SpriteDefinitionRef select_sprite(const char *prompt, const SpriteDefinitionRef &selectedSprite,  bool is_editor, int &exit_status);

enum EnemySelectFlags {
    ESF_GOODGUYS = 1,
    ESF_BADGUYS = 2
};

EnemyDefinitionRef select_enemy(const char *prompt, const EnemyDefinitionRef &selectedEnemy, int enemySelectFlags, bool is_editor, int &exit_status);

// needed to make resizing work correctly
int d_modlist_proc(int msg, DIALOG *d, int c);
int d_en_modlist_proc(int msg, DIALOG *d, int c);
int d_ilist_proc(int msg, DIALOG *d, int c);
int d_wlist_proc(int msg, DIALOG *d, int c);
int d_enelist_proc(int msg, DIALOG *d, int c);

#endif