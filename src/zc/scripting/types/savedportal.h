#ifndef ZC_SCRIPTING_SAVEDPORTAL_H_
#define ZC_SCRIPTING_SAVEDPORTAL_H_

#include <cstdint>

struct savedportal;

savedportal* checkSavedPortal(int32_t ref, bool skiperr = false);
int32_t getPortalFromSaved(savedportal* p);

int32_t savedportal_get_register(int32_t reg);
void savedportal_set_register(int32_t reg, int32_t value);

#endif
