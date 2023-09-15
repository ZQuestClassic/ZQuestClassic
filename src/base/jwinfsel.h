#ifndef _JWIN_FSEL_H_
#define _JWIN_FSEL_H_

#include "jwin.h"


#define FLIST_SIZE      2048
typedef struct FLIST
{
	char dir[1024];
	int32_t size;
	char *name[FLIST_SIZE];
	
	void load(const char* path);
	bool get(int32_t index, char* buf);
	void clear();
} FLIST;

void init_fs_flist(FLIST* list, char* dir);
int32_t get_fs_size(FLIST* list);
bool get_fs_file(FLIST* list, int32_t index, char* buf);
void destroy_fs_flist(FLIST* list);

// #ifdef __cplusplus
// extern "C"
// {
// #endif

/* jwin_file_select:
  *  Displays the Allegro file selector, with the message as caption.
  *  Allows the user to select a file, and stores the selection in path
  *  (which should have room for at least 80 characters). The files are
  *  filtered according to the file extensions in ext. Passing NULL
  *  includes all files, "PCX;BMP" includes only files with .PCX or .BMP
  *  extensions. Returns zero if it was closed with the Cancel button or
  *  if the path has no file name, or non-zero if it was OK'd.
  */
//int32_t jwin_file_select(char *message, char *path, char *ext);
int32_t jwin_file_select_ex(AL_CONST char *message, char *path, AL_CONST char *ext, int32_t size, int32_t width, int32_t height, FONT *title_font);

/* jwin_dfile_select:
  *  Same as jwin_file_select except that it returns TRUE on OK even
  *  if the selected path doesn't include a file name.
  */
//int32_t jwin_dfile_select(char *message, char *path, char *ext);
int32_t jwin_dfile_select_ex(AL_CONST char *message, char *path, AL_CONST char *ext, int32_t size, int32_t width, int32_t height, FONT *title_font);

void get_root_path(char* path, int32_t size);
void relativize_path(char* dest, char const* path);
std::string relativize_path(std::string src_path);
void derelativize_path(char* dest, char const* path);
std::string derelativize_path(std::string src_path);

typedef struct EXT_LIST
{
    const char *text;                                             // appears in drop list
    const char *ext;                                              // same as *ext in file_select()
} EXT_LIST;

/* jwin_file_browse:
  *  Same as jwin_file_select except that it takes a list of possible
  *  extensions and lets the user select from them. The list should be
  *  an array of EXT_LIST structures terminated by one with text==NULL.
  *  list_sel is the index of the current selection in the EXT_LIST.
  */
//int32_t jwin_file_browse(char *message, char *path, EXT_LIST *list, int32_t *list_sel);
int32_t jwin_file_browse_ex(AL_CONST char *message, char *path, EXT_LIST *list, int32_t *list_sel, int32_t size, int32_t width, int32_t height, FONT *title_font);

// #ifdef __cplusplus
// }
// #endif
#endif                                                      // _JWIN_FSEL_H_
