#ifndef _ZINFO_H
#define _ZINFO_H

void assignchar(char** p, char const* str);

struct zinfo
{
	void clear_ic_help();
	void clear_ic_name();
	void clear_ctype_name();
	void clear_ctype_help();
	void clear_mf_name();
	void clear_mf_help();
	void clear();
	zinfo();
#ifdef IS_ZQUEST
	//ZQ Only
	char *ic_help_string[itype_max];
	char *ctype_name[cMAX];
	char *ctype_help_string[cMAX];
	char *mf_name[mfMAX];
	char *mf_help_string[mfMAX];
#else
	//ZC Only
#endif
	//Shared
	char *ic_name[itype_max];
	
	bool isUsableItemclass(size_t q);
	bool isUsableComboType(size_t q);
	bool isUsableMapFlag(size_t q);
	char const* getItemClassName(size_t q);
	char const* getItemClassHelp(size_t q);
	char const* getComboTypeName(size_t q);
	char const* getComboTypeHelp(size_t q);
	char const* getMapFlagName(size_t q);
	char const* getMapFlagHelp(size_t q);
	
	void copyFrom(zinfo const& other);
	bool isNull();
};

extern zinfo ZI;

#ifdef IS_ZQUEST
int32_t writezinfo(PACKFILE *f, zinfo const& z);
#endif
int32_t readzinfo(PACKFILE *f, zinfo& z, zquestheader const& hdr);

#endif

