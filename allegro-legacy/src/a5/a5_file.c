/* libc should use 64-bit for file sizes when possible */
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "allegro.h"
#include "allegro/internal/aintern.h"

/* _al_file_isok:
 *  Helper function to check if it is safe to access a file on a floppy
 *  drive.
 */
int _al_file_isok(const char * filename)
{
    return 1;
}



/* _al_file_size_ex:
 *  Measures the size of the specified file.
 */
uint64_t _al_file_size_ex(const char * filename)
{
    ALLEGRO_FS_ENTRY * ep;
    uint64_t size = 0;

    ep = al_create_fs_entry(filename);
    if(ep)
    {
        size = al_get_fs_entry_size(ep);
        al_destroy_fs_entry(ep);
    }

    return size;
}



/* _al_file_time:
 *  Returns the timestamp of the specified file.
 */
time_t _al_file_time(const char * filename)
{
    ALLEGRO_FS_ENTRY * ep;
    time_t ftime = 0;

    ep = al_create_fs_entry(filename);
    if(ep)
    {
        ftime = al_get_fs_entry_mtime(ep);
        al_destroy_fs_entry(ep);
    }

    return ftime;
}



/* ff_get_filename:
 *  When passed a completely specified file path, this returns a pointer
 *  to the filename portion.
 */
static char *ff_get_filename(const char * path)
{
    char * p = (char *)path + strlen(path);

    // local edit
    while((p > path) && (*(p - 1) != OTHER_PATH_SEPARATOR))
    {
        p--;
    }

    return p;
}



/* ff_put_backslash:
 *  If the last character of the filename is not a /, this routine will
 *  concatenate a / on to it.
 */
static void ff_put_backslash(char * filename, int size)
{
    int len = strlen(filename);

    // local edit
    if((len > 0) && (len < (size - 1)) && (filename[len - 1] != OTHER_PATH_SEPARATOR))
    {
        filename[len] = OTHER_PATH_SEPARATOR;
        filename[len + 1] = 0;
    }
}



#define FF_MATCH_TRY 0
#define FF_MATCH_ONE 1
#define FF_MATCH_ANY 2


struct FF_MATCH_DATA
{
    int type;
    const char * s1;
    const char * s2;
};



/* ff_match:
 *  Matches two strings ('*' matches any number of characters,
 *  '?' matches any character).
 */
static int ff_match(const char * s1, const char * s2)
{
    static unsigned int size = 0;
    static struct FF_MATCH_DATA * data = NULL;
    const char * s1end;
    int index, c1, c2;

    /* handle NULL arguments */
    if((!s1) && (!s2))
    {
        if (data)
        {
            free(data);
            data = NULL;
        }
        return 0;
    }

    s1end = s1 + strlen(s1);

    /* allocate larger working area if necessary */
    if(data && (size < strlen(s2)))
    {
        free(data);
        data = NULL;
    }

    if(!data)
    {
        size = strlen(s2);
        data = malloc(sizeof(struct FF_MATCH_DATA) * size * 2 + 1);
        if(!data)
        {
            return 0;
        }
    }

    index = 0;
    data[0].s1 = s1;
    data[0].s2 = s2;
    data[0].type = FF_MATCH_TRY;

    while(index >= 0)
    {
        s1 = data[index].s1;
        s2 = data[index].s2;
        c1 = *s1;
        c2 = *s2;

        switch(data[index].type)
        {
            case FF_MATCH_TRY:
            {
                if(c2 == 0)
                {
                    /* pattern exhausted */
                    if(c1 == 0)
                    {
                        return 1;
                    }
                    else
                    {
                        index--;
                    }
                }
                else if(c1 == 0)
                {
                    /* string exhausted */
                    while(*s2 == '*')
                    {
                        s2++;
                    }
                    if(*s2 == 0)
                    {
                        return 1;
                    }
                    else
                    {
                        index--;
                    }
                }
                else if(c2 == '*')
                {
                    /* try to match the rest of pattern with empty string */
                    data[index++].type = FF_MATCH_ANY;
                    data[index].s1 = s1end;
                    data[index].s2 = s2 + 1;
                    data[index].type = FF_MATCH_TRY;
                }
                else if((c2 == '?') || (c1 == c2))
                {
                    /* try to match the rest */
                    data[index++].type = FF_MATCH_ONE;
                    data[index].s1 = s1 + 1;
                    data[index].s2 = s2 + 1;
                    data[index].type = FF_MATCH_TRY;
                }
                else
                {
                    index--;
                }
                break;
            }
            case FF_MATCH_ONE:
            {
                /* the rest of string did not match, try earlier */
                index--;
                break;
            }
            case FF_MATCH_ANY:
            {
                /* rest of string did not match, try add more chars to string tail */
                if (--data[index + 1].s1 >= s1)
                {
                    data[index + 1].type = FF_MATCH_TRY;
                    index++;
                }
                else
                {
                    index--;
                }
                break;
            }
            default:
            {
                /* this is a bird? This is a plane? No it's a bug!!! */
                return 0;
            }
        }
    }
    return 0;
}



/* structure for use by the directory scanning routines */
#define FF_MAXPATHLEN 1024

typedef struct
{
    ALLEGRO_FS_ENTRY * root_entry;
    char dirname[FF_MAXPATHLEN];
    char pattern[FF_MAXPATHLEN];
    int attrib;
    uint64_t size;
    int stage;
} FF_DATA;



/* al_findfirst:
 *  Initiates a directory search.
 */
int al_findfirst(const char * pattern, struct al_ffblk * info, int attrib)
{
    FF_DATA * ff_data;
    int a5_attrib = 0;
    char tmp[1024];
    char * p;
    ALLEGRO_FS_ENTRY * entry;

    /* allocate ff_data structure */
    ff_data = malloc(sizeof(FF_DATA));
    if(!ff_data)
    {
        *allegro_errno = ENOMEM;
        return -1;
    }

    memset(ff_data, 0, sizeof(FF_DATA));
    info->ff_data = ff_data;

    /* if the pattern contains no wildcard, we use stat() */
    if(!ustrpbrk(pattern, uconvert("?*", U_ASCII, tmp, U_CURRENT, sizeof(tmp))))
    {
        /* start the search */
        errno = *allegro_errno = 0;

        entry = al_create_fs_entry(pattern);

        if(entry)
        {
            if(!al_fs_entry_exists(entry))
            {
                al_destroy_fs_entry(entry);
                return -1;
            }
            /* does it match ? */
            if(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR)
            {
                a5_attrib |= FA_DIREC;
            }
            if((a5_attrib & ~attrib) == 0)
            {
                if(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR)
                {
                    info->attrib = FA_DIREC;
                }
                else
                {
                    info->attrib = 0;
                }
                info->time = al_get_fs_entry_mtime(entry);
                info->size = al_get_fs_entry_size(entry); /* overflows at 2GB */
                ff_data->size = al_get_fs_entry_size(entry);
                ustrzcpy(info->name, sizeof(info->name), get_filename(pattern));
                al_destroy_fs_entry(entry);
                return 0;
            }
        }
        free(ff_data);
        info->ff_data = NULL;
        *allegro_errno = (errno ? errno : ENOENT);
        return -1;
    }

    ff_data->attrib = attrib;

    do_uconvert(pattern, U_CURRENT, ff_data->dirname, U_UTF8, sizeof(ff_data->dirname));
    p = ff_get_filename(ff_data->dirname);
    _al_sane_strncpy(ff_data->pattern, p, sizeof(ff_data->pattern));
    if(p == ff_data->dirname)
    {
        _al_sane_strncpy(ff_data->dirname, "./", FF_MAXPATHLEN);
    }
    else
    {
        *p = 0;
    }

    /* nasty bodge, but gives better compatibility with DOS programs */
    if(strcmp(ff_data->pattern, "*.*") == 0)
    {
        _al_sane_strncpy(ff_data->pattern, "*", FF_MAXPATHLEN);
    }

    /* start the search */
    errno = *allegro_errno = 0;

    ff_data->root_entry = al_create_fs_entry(ff_data->dirname);

    if(!ff_data->root_entry)
    {
        *allegro_errno = (errno ? errno : ENOENT);
        free(ff_data);
        info->ff_data = NULL;
        return -1;
    }
    if(!al_open_directory(ff_data->root_entry))
    {
        al_destroy_fs_entry(ff_data->root_entry);
        *allegro_errno = (errno ? errno : ENOENT);
        free(ff_data);
        info->ff_data = NULL;
        return -1;
    }

    if(al_findnext(info) != 0)
    {
        al_findclose(info);
        return -1;
    }

    return 0;
}



/* al_findnext:
 *  Retrieves the next file from a directory search.
 */
int al_findnext(struct al_ffblk * info)
{
    char tempname[FF_MAXPATHLEN];
    char filename[FF_MAXPATHLEN];
    ALLEGRO_FS_ENTRY * entry;
    FF_DATA * ff_data = (FF_DATA *)info->ff_data;
    const char* entry_name;
    bool read_fake_entry;

    ASSERT(ff_data);

    /* if the pattern contained no wildcard */
    if(strlen(ff_data->pattern) < 1)
    {
        return -1;
    }

    while(1)
    {
        if (ff_data->stage < 2) {
            // Allegro5 explictly ignores ".." and ".", but allegro4 didn't.
            // see https://github.com/liballeg/allegro5/blob/4aa54e6c994af21bc63d8b593673ab3df62390f8/src/fshook_stdio.c#L421
            if (ff_data->stage == 0) entry_name = "..";
            else entry_name = ".";
            ff_data->stage++;
            read_fake_entry = true;
        } else {
            /* read directory entry */
            entry = al_read_directory(ff_data->root_entry);
            if(!entry)
            {
                *allegro_errno = (errno ? errno : ENOENT);
                return -1;
            }
            // local edit
            if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_HIDDEN)
                continue;
            entry_name = al_get_fs_entry_name(entry);
            read_fake_entry = false;
        }

        /* try to match file name with pattern */
        tempname[0] = 0;
        if(strlen(entry_name) >= sizeof(tempname))
        {
            strncat(tempname, entry_name, sizeof(tempname) - 1);
        }
        else
        {
            strncat(tempname, entry_name, strlen(entry_name));
        }

        // local edit
#ifdef ALLEGRO_LEGACY_WINDOWS
        // The allegro5 file code won't return paths with lowercased drive components,
        // but allegro4 has forced the pattern's drive to be lowercase (see canonicalize_filename).
        // To ensure this doesn't fail to match the pattern, simply do the same lowercasing here.
        tempname[0] = tolower(tempname[0]);
#endif

        if(ff_match(tempname, ff_data->pattern))
        {
            _al_sane_strncpy(filename, ff_data->dirname, FF_MAXPATHLEN);
            ff_put_backslash(filename, sizeof(filename));
            strncat(filename, tempname, sizeof(filename) - strlen(filename) - 1);
            break;
        }
    }

    // local edit
    if(read_fake_entry)
    {
        info->attrib = FA_DIREC;
        do_uconvert(entry_name, U_UTF8, info->name, U_CURRENT, sizeof(info->name));
    }
    else
    {
        if(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR)
        {
            info->attrib = FA_DIREC;
        }
        else
        {
            info->attrib = 0;
        }
        info->time = al_get_fs_entry_mtime(entry);
        info->size = al_get_fs_entry_size(entry); /* overflows at 2GB */
        ff_data->size = al_get_fs_entry_size(entry);
        do_uconvert(tempname, U_UTF8, info->name, U_CURRENT, sizeof(info->name));
        al_destroy_fs_entry(entry);
    }

    return 0;
}



/* al_findclose:
 *  Cleans up after a directory search.
 */
void al_findclose(struct al_ffblk * info)
{
    FF_DATA * ff_data = (FF_DATA *) info->ff_data;

    if(ff_data)
    {
        if(ff_data->root_entry)
        {
            al_close_directory(ff_data->root_entry);
            al_destroy_fs_entry(ff_data->root_entry);
        }
        free(ff_data);
        info->ff_data = NULL;

        /* to avoid leaking memory */
        ff_match(NULL, NULL);
    }
}



/* _al_getdcwd:
 *  Returns the current directory on the specified drive.
 */
void _al_getdcwd(int drive, char * buf, int size)
{
    char tmp[1024];

    if(getcwd(tmp, sizeof(tmp)))
    {
        do_uconvert(tmp, U_UTF8, buf, U_CURRENT, size);
    }
    else
    {
        usetc(buf, 0);
    }
}



/* _al_ffblk_get_size:
 *  Returns the size out of an _al_ffblk structure.
 */
uint64_t al_ffblk_get_size(struct al_ffblk * info)
{
    FF_DATA * ff_data;
    ASSERT(info);
    ff_data = (FF_DATA *) info->ff_data;
    ASSERT(ff_data);
    return ff_data->size;
}



/* _al_detect_filename_encoding:
 *  Platform specific function to detect the filename encoding. This is called
 *  after setting a system driver, and even if this driver is SYSTEM_NONE.
 */
void _al_detect_filename_encoding(void)
{
    char * locale = getenv("LC_ALL");

    if(!locale || !locale[0])
    {
        locale = getenv("LC_CTYPE");
        if(!locale || !locale[0])
        {
            locale = getenv("LANG");
        }
    }

    if(locale)
    {
        if(strstr(locale, "utf8") || strstr(locale, "UTF-8") || strstr(locale, "utf-8") ||  strstr(locale, "UTF8"))
        {
            /* Note: UTF8 is default anyway. */
            set_filename_encoding(U_UTF8);
        }
        /* TODO: detect other encodings, and support them in Allegro */
    }
}

#ifdef ALLEGRO_LEGACY_WINDOWS

    /* _al_getdrive:
     *  Returns the current drive number (0=A, 1=B, etc).
     */
    int _al_getdrive(void)
    {
       return _getdrive() - 1;
    }



    /* _al_win_open:
     *  Open a file with open() or _wopen() depending on whether Unicode filenames
     *  are supported by this version of Windows and compiler.
     */
    int _al_win_open(const char *filename, int mode, int perm)
    {
       if (get_filename_encoding() != U_UNICODE) {
          return open(filename, mode, perm);
       }
       else {
          return _wopen((wchar_t*)filename, mode, perm);
       }
    }



    /* _al_win_unlink:
     *  Remove a file with unlink() or _wunlink() depending on whether Unicode
     *  filenames are supported by this version of Windows and compiler.
     */
    int _al_win_unlink(const char *pathname)
    {
       if (get_filename_encoding() != U_UNICODE) {
          return unlink(pathname);
       }
       else {
          return _wunlink((wchar_t*)pathname);
       }
    }

#endif
