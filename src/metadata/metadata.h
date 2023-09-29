#include "metadata/sigs/devsig.h.sig"
#include "metadata/sigs/compilersig.h.sig"
#include "metadata/versionsig.h"

//{ Version string info
#define STRGZ_2(s) #s
#define STRGZ(s) STRGZ_2(s)

#if V_ZC_RELEASE > 0
#define ALPHA_SUB_STR "Release"
#define ALPHA_VER V_ZC_RELEASE
#define ALPHA_STATE 3

#elif V_ZC_GAMMA > 0
#define ALPHA_SUB_STR "Gamma"
#define ALPHA_VER V_ZC_GAMMA
#define ALPHA_STATE 2

#elif V_ZC_BETA > 0
#define ALPHA_SUB_STR "Beta"
#define ALPHA_VER V_ZC_BETA
#define ALPHA_STATE 1

#elif V_ZC_ALPHA > 0
#define ALPHA_SUB_STR "Alpha"
#define ALPHA_VER V_ZC_ALPHA
#define ALPHA_STATE 0

#else
#define ALPHA_SUB_STR "Unknown"
#define NIL_VER 0
#define ALPHA_VER NIL_VER
#define ALPHA_STATE -1
#endif

#if ZC_IS_NIGHTLY > 0
#define ALPHA_STR ("Nightly (" ALPHA_SUB_STR ")")
#define ALPHA_VER_STR ("Nightly (" ALPHA_SUB_STR " " + std::to_string(ALPHA_VER-1) + "/" STRGZ(ALPHA_VER) ")").c_str()
#else
#define ALPHA_STR (ALPHA_SUB_STR)
#define ALPHA_VER_STR (ALPHA_SUB_STR " " STRGZ(ALPHA_VER))
#endif

#define COMPANY_NAME "Armageddon Games"
#define ZQ_EDITOR_NAME "ZQuest Classic Editor"
#define ZQ_EDITOR_V "2.55"
#define ZQ_EDITOR_COPYRIGHT "Copyright (C) 2021"

#define ZC_PLAYER_NAME "ZQuest Classic Player"
#define ZC_PLAYER_V "2.55"
#define ZC_PLAYER_COPYRIGHT "Copyright (C) 2021"
//}
