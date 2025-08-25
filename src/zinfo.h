#ifndef ZINFO_H_
#define ZINFO_H_
#include "base/mapscr.h"

void assignchar(char** p, char const* str);

struct zinfo
{
	void clear_ic_help();
	void clear_ic_name();
	void clear_ctype_name();
	void clear_ctype_help();
	void clear_mf_name();
	void clear_mf_help();
	void clear_ctr_name();
	void clear_weap_name();
	void clear_etype_name();
	void clear_li_name();
	void clear_li_help();
	void clear_li_abbr();
	void clear();
	zinfo();

	//ZQ Only
	char *ic_help_string[itype_max];
	char *litem_help_string[li_max];
	char *ctype_name[cMAX];
	char *ctype_help_string[cMAX];
	char *mf_name[mfMAX];
	char *mf_help_string[mfMAX];
	char *weap_name[wMax];
	char *etype_name[eeMAX];

	//Shared
	char *ic_name[itype_max];
	char *ctr_name[MAX_COUNTERS];
	char *litem_name[li_max];
	char *litem_abbr[li_max];
	
	bool isUsableItemclass(size_t q);
	bool isUsableComboType(size_t q);
	bool isUsableMapFlag(size_t q);
	bool isUsableWeap(size_t q);
	bool isUsableCtr(int32_t q);
	bool isUsableEnemyType(size_t q);
	char const* getItemClassName(size_t q);
	char const* getItemClassHelp(size_t q);
	char const* getComboTypeName(size_t q);
	char const* getComboTypeHelp(size_t q);
	char const* getMapFlagName(size_t q);
	char const* getWeapName(size_t q);
	char const* getMapFlagHelp(size_t q);
	char const* getCtrName(int32_t q);
	char const* getEnemyTypeName(size_t q);
	char const* getLevelItemName(size_t q);
	char const* getLevelItemHelp(size_t q);
	char const* getLevelItemAbbr(size_t q);
	
	void copyFrom(zinfo const& other);
	bool isNull();
};

extern zinfo ZI;

int32_t writezinfo(PACKFILE *f, zinfo const& z);
int32_t readzinfo(PACKFILE *f, zinfo& z, zquestheader const& hdr);

#endif
