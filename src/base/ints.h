#ifndef _INTS_H_
#define _INTS_H_

#include <stdint.h>
#include <cstring>

typedef uint8_t  byte;  //0-255  ( 8 bits)
typedef uint16_t word;  //0-65,535  (16 bits)
typedef uint32_t dword; //0-4,294,967,295  (32 bits)
typedef uint64_t qword; //0-18,446,744,073,709,551,616  (64 bits)

typedef unsigned const char ucc;

using std::size_t;

#endif
