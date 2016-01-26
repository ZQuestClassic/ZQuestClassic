#ifndef _ANGELSCRIPT_ASZC_H_
#define _ANGELSCRIPT_ASZC_H_

class enemy;
class item;
class weapon;

void initializeAngelScript();
void shutDownAngelScript();
void assignEnemyScript(enemy* en, const char* scriptName);
void assignItemScript(item* it, const char* scriptName);
void assignWeaponScript(weapon* w, const char* scriptName);

#endif
