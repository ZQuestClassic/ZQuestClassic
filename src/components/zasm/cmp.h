#ifndef ZASM_CMP_H_
#define ZASM_CMP_H_

#include "base/ints.h"
#include <string>

// Comparison flags for the ZASM compare-and-branch commands (GOTOCMP,
// SETCMP, ...).

// Base flags, describing the relation
#define CMP_GT    0x01
#define CMP_LT    0x02
#define CMP_EQ    0x04
// Extra behavior flags
#define CMP_SETI  0x08 // In 'SETCMP', multiplies the output by 10000.
#define CMP_BOOL  0x10 // Does a "boolean comparison" (ex. 2 CMP_EQ 3 is true, since !2 == !3)

#define CMP_FLAGS 0x07
#define CMP_NE    (CMP_GT|CMP_LT)
#define CMP_GE    (CMP_GT|CMP_EQ)
#define CMP_LE    (CMP_LT|CMP_EQ)

#define INVERT_CMP(cmp) ((cmp&(~CMP_FLAGS))|((~cmp)&CMP_FLAGS))
inline std::string CMP_STR(uint cmpval)
{
	std::string pref;
	if(cmpval & CMP_BOOL)
		pref += "B";
	if(cmpval & CMP_SETI)
		pref += "I";
	switch(cmpval&CMP_FLAGS)
	{
		default:
			return pref+"??";
		case 0:
			return pref+"Never";
		case CMP_GT:
			return pref+">";
		case CMP_GE:
			return pref+">=";
		case CMP_LT:
			return pref+"<";
		case CMP_LE:
			return pref+"<=";
		case CMP_EQ:
			return pref+"==";
		case CMP_NE:
			return pref+"!=";
		case CMP_FLAGS:
			return pref+"Always";
	}
}

#endif
