#pragma once

#include <cstdint>

struct savedportal;

savedportal* checkSavedPortal(int32_t ref, bool skiperr = false);
int32_t getPortalFromSaved(savedportal* p);

int32_t savedportal_get_register(int32_t reg);
void savedportal_set_register(int32_t reg, int32_t value);
