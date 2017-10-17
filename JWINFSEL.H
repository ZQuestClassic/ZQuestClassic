/*                 __                  __
  *                /_/\  __  __  __    /_/\  ______
  *               _\_\/ / /\/ /\/ /\  _\_\/ / ____ \
  *              / /\  / / / / / / / / /\  / /\_ / /\
  *         __  / / / / /_/ /_/ / / / / / / / / / / /
  *        / /_/ / / /_________/ / /_/ / /_/ / /_/ /
  *        \____/ /  \_________\/  \_\/  \_\/  \_\/
  *         \___\/
  *
  *
  *
  *     jwinfsel.h
  *
  *     The file selector.
  *
  *     By Shawn Hargreaves.
  *
  *     Guilherme Silveira and Theuzifan Sumachingun both independently
  *     modified it to only list valid drive letters.
  *
  *     Adapted to "jwin" style by Jeremy Craner. Also added the "browser".
  *
  *     Version: 3/22/00
  *     Allegro version: 3.1x  (don't know if it works with WIP)
  *
  */

#ifndef _JWIN_FSEL_H_
#define _JWIN_FSEL_H_

#include "jwin.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* jwin_file_select:
    *  Displays the Allegro file selector, with the message as caption.
    *  Allows the user to select a file, and stores the selection in path
    *  (which should have room for at least 80 characters). The files are
    *  filtered according to the file extensions in ext. Passing NULL
    *  includes all files, "PCX;BMP" includes only files with .PCX or .BMP
    *  extensions. Returns zero if it was closed with the Cancel button or
    *  if the path has no file name, or non-zero if it was OK'd.
    */
  //int jwin_file_select(char *message, char *path, char *ext);
  int jwin_file_select_ex(AL_CONST char *message, char *path, AL_CONST char *ext, int size, int width, int height, FONT *title_font);

  /* jwin_dfile_select:
    *  Same as jwin_file_select except that it returns TRUE on OK even
    *  if the selected path doesn't include a file name.
    */
  //int jwin_dfile_select(char *message, char *path, char *ext);
  int jwin_dfile_select_ex(AL_CONST char *message, char *path, AL_CONST char *ext, int size, int width, int height, FONT *title_font);

  typedef struct EXT_LIST
  {
    char *text;                                             // appears in drop list
    char *ext;                                              // same as *ext in file_select()
  } EXT_LIST;

  /* jwin_file_browse:
    *  Same as jwin_file_select except that it takes a list of possible
    *  extensions and lets the user select from them. The list should be
    *  an array of EXT_LIST structures terminated by one with text==NULL.
    *  list_sel is the index of the current selection in the EXT_LIST.
    */
  //int jwin_file_browse(char *message, char *path, EXT_LIST *list, int *list_sel);
  int jwin_file_browse_ex(AL_CONST char *message, char *path, EXT_LIST *list, int *list_sel, int size, int width, int height, FONT *title_font);

#ifdef __cplusplus
}
#endif
#endif                                                      // _JWIN_FSEL_H_
 
