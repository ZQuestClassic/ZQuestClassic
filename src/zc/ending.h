#ifndef ENDING_H_
#define ENDING_H_

#include <cstdint>

void noproc();
void put_triforce();
void putendmsg(const char *s,int32_t x,int32_t y,int32_t speed,void(proc)());
void brick(int32_t x,int32_t y);
void endingpal();
void ending();
void ending_scripted();
#endif
 
