#ifndef _ZINFO_H
#define _ZINFO_H

void assignchar(char** p, char const* str);

struct zinfo
{
	void clear_ic_help();
	void clear_ic_name();
	void clear();
	zinfo();
#ifdef IS_ZQUEST
	//ZQ Only
	char *ic_help_string[itype_max];
#else
	//ZC Only
#endif
	//Shared
	char *ic_name[itype_max];
	
	bool zinfo::isUsableItemclass(size_t q);
	char const* zinfo::getItemClassName(size_t q);
	char const* zinfo::getItemClassHelp(size_t q);
	
	void copyFrom(zinfo const& other);
	bool isNull();
};

extern zinfo ZI;

#ifdef IS_ZQUEST
int32_t writezinfo(PACKFILE *f, zinfo const& z);
#endif
int32_t readzinfo(PACKFILE *f, zinfo& z);

#endif

