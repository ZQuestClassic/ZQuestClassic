#include "base/files.h"
#ifdef __GNUG__
	#define ALLEGRO_NO_FIX_ALIASES
#endif

#include <cstring>
#include <filesystem>

#include "base/zc_alleg.h"
#include <allegro/alcompat.h>

#include "gui/jwin.h"
#include "base/jwinfsel.h"
#include "base/zsys.h"
#include "base/fonts.h"

namespace fs = std::filesystem;

#if (DEVICE_SEPARATOR != 0) && (DEVICE_SEPARATOR != '\0')
#define HAVE_DIR_LIST
#endif


static int32_t fs_edit_proc(int32_t, DIALOG *, int32_t);
static int32_t fs_flist_proc(int32_t, DIALOG *, int32_t);
static int32_t fs_elist_proc(int32_t, DIALOG *, int32_t);
static const char *fs_flist_getter(int32_t, int32_t *);
static const char *fs_elist_getter(int32_t, int32_t *);

#ifdef HAVE_DIR_LIST

static int32_t fs_dlist_proc(int32_t, DIALOG *, int32_t);
static const char *fs_dlist_getter(int32_t, int32_t *);
#endif

static FLIST *flist = NULL;

/* file extensions */
static char *fext = NULL;                                   /* tokenized extension string (dynamically allocated)     */
static EXT_LIST *fext_list = NULL;
static char **fext_p = NULL;                                /* list of pointers to the tokens (dynamically allocated) */
static int32_t fext_size = 0;                                   /* size of the list                                       */

/* file attributes (rhsda order) */
#define ATTRB_MAX     5                                     /* number of attributes */
#define ATTRB_DIREC   3                                     /* index of FA_DIREC    */

typedef enum { ATTRB_ABSENT, ATTRB_UNSET, ATTRB_SET }
attrb_state_t;

#define DEFAULT_ATTRB_STATE  { ATTRB_ABSENT, ATTRB_UNSET, ATTRB_UNSET, ATTRB_ABSENT, ATTRB_ABSENT }

static int32_t attrb_flag[ATTRB_MAX] = { FA_RDONLY, FA_HIDDEN, FA_SYSTEM, FA_DIREC, FA_ARCH };
static attrb_state_t attrb_state[ATTRB_MAX] = DEFAULT_ATTRB_STATE;

static char updir[1024];

static int32_t fs_dummy_proc(int32_t msg, DIALOG *d, int32_t c)
{
    //these are here to bypass compiler warnings about unused arguments
    msg=msg;
    d=d;
    c=c;
    
    return D_O_K;
}

static ListData fs_flist__getter(fs_flist_getter, &font);
static ListData fs_elist__getter(fs_elist_getter, &font);
#ifdef HAVE_DIR_LIST //Needed to compile. -L
static ListData fs_dlist__getter(fs_dlist_getter, &font);
#endif

static DIALOG file_selector[] =
{
#ifdef HAVE_DIR_LIST

    /* (dialog proc)        (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)                       (dp2) (dp3) */
    { jwin_win_proc,        0,    0,    305,  161,  0,    0,    0,    D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { jwin_button_proc,     208,  107,  81,   17,   0,    0,    0,    D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { jwin_button_proc,     208,  129,  81,   17,   0,    0,    27,   D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { fs_edit_proc,         16,   28,   272,  8,    0,    0,    0,    0,       79,   0,    NULL,                      NULL, NULL  },
    { fs_elist_proc,        16,   154,  176,  16,   0,    0,    0,    D_EXIT,  0,    0, (void *) &fs_elist__getter,  NULL, NULL },
    { fs_flist_proc,        16,   46,   177,  100,  0,    0,    0,    D_EXIT,  0,    0, (void *) &fs_flist__getter,  NULL, NULL  },
    { jwin_button_proc,     250,  160,  81,   17,   0,    0,    27,   D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { fs_dlist_proc,        208,  46,   81,   52,   0,    0,    0,    D_EXIT,  0,    0, (void *) &fs_dlist__getter,  NULL, NULL  },
    { d_yield_proc,         0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,                      NULL, NULL  },
    
#else
    
    /* (dialog proc)        (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)                       (dp2) (dp3) */
    { jwin_win_proc,        0,    0,    305,  189,  0,    0,    0,    D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { jwin_button_proc,     64,   160,  81,   17,   0,    0,    0,    D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { jwin_button_proc,     160,  160,  81,   17,   0,    0,    27,   D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { fs_edit_proc,         16,   28,   272,  8,    0,    0,    0,    0,       79,   0,    NULL,                      NULL, NULL  },
    { fs_elist_proc,        16,   154,  176,  16,   0,    0,    0,    D_EXIT,  0,    0, (void *) &fs_elist__getter,  NULL, NULL },
    { fs_flist_proc,        16,   46,   273,  100,  0,    0,    0,    D_EXIT,  0,    0, (void *) &fs_flist__getter,  NULL, NULL  },
    { jwin_button_proc,     250,  160,  81,   17,   0,    0,    27,   D_EXIT,  0,    0,    NULL,                      NULL, NULL  },
    { d_yield_proc,         0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,                      NULL, NULL  },
#endif
    
    { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

#define FS_WIN          0
#define FS_OK           1
#define FS_CANCEL       2
#define FS_EDIT         3
#define FS_TYPES        4
#define FS_FILES        5
#define FS_EXPLORER     6

#ifdef __APPLE__
#define EXPLORER_LABEL "Finder"
#else
#define EXPLORER_LABEL "File Explorer"
#endif

#ifdef HAVE_DIR_LIST                                        /* not all platforms need a directory list */

#define FS_DISKS        7
#define FS_YIELD        8

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

/* count_disks:
  *  Counts the number of valid drives.
  */
static int32_t count_disks(void)
{
    int32_t c, i;
    
    c = 0;
    
    for(i=0; i<26; i++)
        if(_al_drive_exists(i))
            c++;
            
    return c;
}

/* get_x_drive:
  *  Returns the drive letter matching the specified list index.
  */
static int32_t get_x_drive(int32_t index)
{
    int32_t c, i;
    
    c = 0;
    
    for(i=0; i<26; i++)
    {
        if(_al_drive_exists(i))
        {
            c++;
            
            if(c==index)
                return i;
        }
    }
    
    return -1;
}

/* fs_dlist_getter:
  *  Listbox data getter routine for the file selector disk list.
  */
static const char *fs_dlist_getter(int32_t index, int32_t *list_size)
{
    static char d[8];
    int32_t pos, c;
    
    if(index < 0)
    {
        if(list_size)
            *list_size = count_disks();
            
        return NULL;
    }
    
    c = 'A' + get_x_drive(index+1);
    
    if((c < 'A') || (c > 'Z'))
        c = 'C';
        
    pos = usetc(d, c);
    pos += usetc(d+pos, DEVICE_SEPARATOR);
    pos += usetc(d+pos, OTHER_PATH_SEPARATOR);
    usetc(d+pos, 0);
    
    return d;
}

/* fs_dlist_proc:
  *  Dialog procedure for the file selector disk list.
  */
static int32_t fs_dlist_proc(int32_t msg, DIALOG *d, int32_t c)
{
    char *s = (char *) file_selector[FS_EDIT].dp;
    int32_t ret, i, temp;
    
    if(msg == MSG_START)
    {
        d->d1 = d->d2 = 0;
        temp = utoupper(ugetc(s));
        
        if(((temp >= 'A') && (temp <= 'Z')) && (ugetat(s, 1) == DEVICE_SEPARATOR))
        {
            temp -= 'A';
            
            for(i=0; i<temp; i++)
                if(_al_drive_exists(i))
                    d->d1++;
        }
    }
    
    ret = jwin_droplist_proc(msg, d, c);
    
    if(ret & D_CLOSE)
    {
        temp = 'A' + get_x_drive(d->d1+1);
        
        if((temp < 'A') || (temp > 'Z'))
            temp = 'C';
            
        s += usetc(s, temp);
        s += usetc(s, DEVICE_SEPARATOR);
        s += usetc(s, OTHER_PATH_SEPARATOR);
        usetc(s, 0);
        
        object_message(file_selector+FS_FILES, MSG_START, 0);
        object_message(file_selector+FS_FILES, MSG_DRAW, 0);
        object_message(file_selector+FS_EDIT, MSG_START, 0);
        object_message(file_selector+FS_EDIT, MSG_DRAW, 0);
        
        return ret - D_CLOSE + D_O_K;
    }
    
    return ret;
}

#else

#define FS_YIELD        7
#endif                                                      /* HAVE_DIR_LIST */

/* fs_edit_proc:
  *  Dialog procedure for the file selector editable string.
  */
static int32_t fs_edit_proc(int32_t msg, DIALOG *d, int32_t c)
{
    char *s = (char *) d->dp;
    int32_t size = (d->d1 + 1) * uwidth_max(U_CURRENT);           /* of s (in bytes) */
    int32_t list_size;
    int32_t found = 0;
    char b[1024], tmp[16];
    int32_t ch, attr;
    int32_t i;
    
    if(msg == MSG_START)
    {
        canonicalize_filename(b, s, sizeof(b));
        ustrzcpy(s, size, b);
    }
    
    if(msg == MSG_KEY)
    {
        if((!ugetc(s)) || (ugetat(s, -1) == DEVICE_SEPARATOR))
            ustrzcat(s, size, uconvert_ascii("./", tmp));
            
        canonicalize_filename(b, s, sizeof(b));
        ustrzcpy(s, size - ucwidth(OTHER_PATH_SEPARATOR), b);
        
        ch = ugetat(s, -1);
        
        if((ch != '/') && (ch != OTHER_PATH_SEPARATOR))
        {
            if(file_exists(s, FA_RDONLY | FA_HIDDEN | FA_DIREC, &attr))
            {
                if(attr & FA_DIREC)
                    put_backslash(s);
                else
                    return D_CLOSE;
            }
            else
                return D_CLOSE;
        }
        
        object_message(file_selector+FS_FILES, MSG_START, 0);
        
        /* did we `cd ..' ? */
        if(ustrlen(updir))
        {
            /* now we have to find a directory name equal to updir */
            for(i = 0; i<flist->size; i++)
            {
                if(!ustrcmp(updir, flist->name[i]))                 /* we got it ! */
                {
                    file_selector[FS_FILES].d1 = i;
                    /* we have to know the number of visible lines in the filelist */
                    /* -1 to avoid an off-by-one problem */
                    list_size = (file_selector[FS_FILES].h-4) / text_height(font) - 1;
                    
                    if(i>list_size)
                        file_selector[FS_FILES].d2 = i-list_size;
                    else
                        file_selector[FS_FILES].d2 = 0;
                        
                    found = 1;
                    break;                                            /* ok, our work is done... */
                }
            }
            
            /* by some strange reason, we didn't find the old directory... */
            if(!found)
            {
                file_selector[FS_FILES].d1 = 0;
                file_selector[FS_FILES].d2 = 0;
            }
        }
        
        /* and continue... */
        object_message(file_selector+FS_FILES, MSG_DRAW, 0);
        object_message(d, MSG_START, 0);
        object_message(d, MSG_DRAW, 0);
        
        return D_O_K;
    }
    
    int32_t allegro_lfn = ALLEGRO_LFN; //removes compiler warning
    
    if(msg == MSG_UCHAR)
    {
        if((c >= 'a') && (c <= 'z'))
        {
            if(!allegro_lfn)
                c = utoupper(c);
        }
        else if(c == '/')
        {
            c = OTHER_PATH_SEPARATOR;
        }
        else if(allegro_lfn)
        {
            if((c > 127) || (c < 32))
                return D_O_K;
        }
        else
        {
            if((c != OTHER_PATH_SEPARATOR) && (c != '_') &&
                    (c != DEVICE_SEPARATOR) && (c != '.') &&
                    ((c < 'A') || (c > 'Z')) && ((c < '0') || (c > '9')))
                return D_O_K;
        }
    }
    
    //   return _gui_edit_proc(msg, d, c);
    return jwin_edit_proc(msg, d, c);
}

/* ustrfilecmp:
  *  ustricmp for filenames: makes sure that eg "foo.bar" comes before
  *  "foo-1.bar", and also that "foo9.bar" comes before "foo10.bar".
  */
static int32_t ustrfilecmp(AL_CONST char *s1, AL_CONST char *s2)
{
    int32_t c1, c2;
    int32_t x1, x2;
    char *t1, *t2;
    
    for(;;)
    {
        c1 = utolower(ugetxc(&s1));
        c2 = utolower(ugetxc(&s2));
        
        if((c1 >= '0') && (c1 <= '9') && (c2 >= '0') && (c2 <= '9'))
        {
            x1 = ustrtol(s1 - ucwidth(c1), &t1, 10);
            x2 = ustrtol(s2 - ucwidth(c2), &t2, 10);
            
            if(x1 != x2)
                return x1 - x2;
            else if(t1 - s1 != t2 - s2)
                return (t2 - s2) - (t1 - s1);
                
            s1 = t1;
            s2 = t2;
        }
        else if(c1 != c2)
        {
            if(!c1)
                return -1;
            else if(!c2)
                return 1;
            else if(c1 == '.')
                return -1;
            else if(c2 == '.')
                return 1;
                
            return c1 - c2;
        }
        
        if(!c1)
            return 0;
    }
}

/* fs_flist_putter:
  *  Callback routine for for_each_file() to fill the file selector listbox.
  */
static int32_t fs_flist_putter(AL_CONST char *str, int32_t attrib, void *check_attrib)
{
    char *s, *ext, *name;
    int32_t c, c2;
    
    s = get_filename(str);
    fix_filename_case(s);
    
    if(!(attrib & FA_DIREC))
    {
        /* Check if file extension matches. */
        if(fext_p)
        {
            ext = get_extension(s);
            
            for(c=0; c<fext_size; c++)
            {
                if(ustricmp(ext, fext_p[c]) == 0)
                    goto Next;
            }
            
            return 0;
        }
        
Next:

        /* Check if file attributes match. */
        if(check_attrib)
        {
            for(c=0; c<ATTRB_MAX; c++)
            {
                if((attrb_state[c] == ATTRB_SET) && (!(attrib & attrb_flag[c])))
                    return 0;
                    
                if((attrb_state[c] == ATTRB_UNSET) && (attrib & attrb_flag[c]))
                    return 0;
            }
        }
    }
    
    if((flist->size < FLIST_SIZE) && ((ugetc(s) != '.') || (ugetat(s, 1))))
    {
        int32_t size = ustrsizez(s) + ((attrib & FA_DIREC) ? ucwidth(OTHER_PATH_SEPARATOR) : 0);
        name = (char *) malloc(size);
        
        if(!name)
            return -1;
            
        ustrzcpy(name, size, s);
        
        if(attrib & FA_DIREC)
            put_backslash(name);
            
        /* Sort alphabetically with directories first. */
        for(c=0; c<flist->size; c++)
        {
            if(ugetat(flist->name[c], -1) == OTHER_PATH_SEPARATOR)
            {
                if(attrib & FA_DIREC)
                    if(ustrfilecmp(name, flist->name[c]) < 0)
                        break;
            }
            else
            {
                if(attrib & FA_DIREC)
                    break;
                    
                if(ustrfilecmp(name, flist->name[c]) < 0)
                    break;
            }
        }
        
        /* Shift in preparation for inserting the new entry. */
        for(c2=flist->size; c2>c; c2--)
            flist->name[c2] = flist->name[c2-1];
            
        /* Insert the new entry. */
        flist->name[c] = name;
        flist->size++;
    }
    
    return 0;
}

/* fs_flist_getter:
  *  Listbox data getter routine for the file selector list.
  */
static const char *fs_flist_getter(int32_t index, int32_t *list_size)
{
    if(index < 0)
    {
        if(list_size)
            *list_size = flist->size;
            
        return NULL;
    }
    
    return flist->name[index];
}

/* build_attrb_flag:
  *  Returns the cumulative flag for all attributes in state STATE.
  */
static int32_t build_attrb_flag(attrb_state_t state)
{
    int32_t i, flag = 0;
    
    for(i = 0; i < ATTRB_MAX; i++)
    {
        if(attrb_state[i] == state)
            flag |= attrb_flag[i];
    }
    
    return flag;
}

/* fs_flist_proc:
  *  Dialog procedure for the file selector list.
  */
static int32_t fs_flist_proc(int32_t msg, DIALOG *d, int32_t c)
{
    static int32_t recurse_flag = 0;
    char *s = (char *) file_selector[FS_EDIT].dp;
    char tmp[32];
    /* of s (in bytes) */
    int32_t size = (file_selector[FS_EDIT].d1 + 1) * uwidth_max(U_CURRENT);
    int32_t sel = d->d1;
    int32_t i, ret;
    int32_t ch, count;
    
    if(msg == MSG_START)
    {
        if(!flist)
        {
            flist = (FLIST *) calloc(1, sizeof(FLIST));
            
            if(!flist)
            {
                *allegro_errno = ENOMEM;
                return D_CLOSE;
            }
        }
        else
        {
            for(i=0; i<flist->size; i++)
                if(flist->name[i])
                    free(flist->name[i]);
        }
        
        flist->size = 0;

        std::error_code ec;
        if (fs::is_directory(s, ec))
            snprintf(flist->dir, 1024, "%s%c*.*", s, fs::path::preferred_separator);
        else
            replace_filename(flist->dir, s, uconvert_ascii("*.*", tmp), sizeof(flist->dir));
        
        /* The semantics of the attributes passed to file_select_ex() is
          * different from that of for_each_file_ex() in one case: when
          * the 'd' attribute is not mentioned in the set of characters,
          * the other attributes are not taken into account for directories,
          * i.e the directories are all included. So we can't filter with
          * for_each_file_ex() in that case.
          */
        if(attrb_state[ATTRB_DIREC] == ATTRB_ABSENT)
            /* accept all dirs */
            for_each_file_ex(flist->dir, 0 , FA_LABEL, fs_flist_putter, (void *)1UL /* check */);
        else
            /* don't check */
            for_each_file_ex(flist->dir, build_attrb_flag(ATTRB_SET), build_attrb_flag(ATTRB_UNSET) | FA_LABEL, fs_flist_putter, (void *)0UL);
            
        usetc(get_filename(flist->dir), 0);
        d->d1 = d->d2 = 0;
        sel = 0;
    }
    
    if(msg == MSG_END)
    {
        if(flist)
        {
            for(i=0; i<flist->size; i++)
                if(flist->name[i])
                    free(flist->name[i]);
                    
            free(flist);
            flist = NULL;
        }
    }
    
    recurse_flag++;
    ret = jwin_abclist_proc(msg,d,c);                         /* call the parent procedure */
    
    recurse_flag--;
    
    if(((sel != d->d1) || (ret == D_CLOSE)) && (recurse_flag == 0))
    {
        replace_filename(s, flist->dir, flist->name[d->d1], size);
        
        /* check if we want to `cd ..' */
        if((!ustrncmp(flist->name[d->d1], uconvert_ascii("..", tmp), 2)) && (ret == D_CLOSE))
        {
            /* let's remember the previous directory */
            usetc(updir, 0);
            i = ustrlen(flist->dir);
            count = 0;
            
            while(i>0)
            {
                ch = ugetat(flist->dir, i);
                
                if((ch == '/') || (ch == OTHER_PATH_SEPARATOR))
                {
                    if(++count == 2)
                        break;
                }
                
                uinsert(updir, 0, ch);
                i--;
            }
            
            /* ok, we have the dirname in updir */
        }
        else
        {
            usetc(updir, 0);
        }
        
        object_message(file_selector+FS_EDIT, MSG_START, 0);
        object_message(file_selector+FS_EDIT, MSG_DRAW, 0);
        
        if(ret == D_CLOSE)
            return object_message(file_selector+FS_EDIT, MSG_KEY, 0);
    }
    
    return ret;
}

/* parse_extension_string:
  *  Parses the extension string, possibly containing attribute characters.
  */
static void parse_extension_string(AL_CONST char *ext)
{
    attrb_state_t state;
    char ext_tokens[32], attrb_char[32];
    char *last, *p, *attrb_p;
    int32_t c, c2, i;
    
    i = 0;
    fext_size = 0;
    fext_p = NULL;
    attrb_p = NULL;
    
    if(!ext)
        return;
        
    fext = ustrdup(ext);
    
    /* Tokenize the extension string and record the pointers to the
      * beginning of each token in a dynamically growing array.
      * ???? We rely on the implementation of ustrtok_r() which writes
      * null characters in the string to delimit the tokens. Yuck.
      */
    c = usetc(ext_tokens, ' ');
    c += usetc(ext_tokens+c, ',');
    c += usetc(ext_tokens+c, ';');
    usetc(ext_tokens+c, 0);
    
    p = ustrtok_r(fext, ext_tokens, &last);
    
    if(p == NULL || !ugetc(p))
        return;
        
    do
    {
        /* Set of attribute characters. */
        if(ugetc(p) == '/')
        {
            attrb_p = p + ucwidth('/');
            continue;
        }
        
        /* Dynamically grow the array if needed. */
        if(i >= fext_size)
        {
            fext_size = (fext_size ? fext_size*2 : 2);
            fext_p = (char **)_al_sane_realloc(fext_p, fext_size * sizeof(char *));
        }
        
        /* Record a pointer to the beginning of the token. */
        fext_p[i++] = p;
        
    }
    while((p = ustrtok_r(NULL, ext_tokens, &last))!=NULL);
    
    /* This is the meaningful size now. */
    fext_size = i;
    
    if(attrb_p)
    {
        state = ATTRB_SET;
        c = usetc(attrb_char, 'r');
        c += usetc(attrb_char+c, 'h');
        c += usetc(attrb_char+c, 's');
        c += usetc(attrb_char+c, 'd');
        c += usetc(attrb_char+c, 'a');
        c += usetc(attrb_char+c, '+');
        c += usetc(attrb_char+c, '-');
        usetc(attrb_char+c, 0);
        
        /* Scan the string. */
        while((c = utolower(ugetx(&attrb_p)))!=0)
        {
            p = attrb_char;
            
            for(i = 0; (c2 = ugetx(&p))!=0; i++)
            {
                if(c == c2)
                {
                    if(i < ATTRB_MAX)
                        attrb_state[i] = state;
                    else
                        state = (i == ATTRB_MAX) ? ATTRB_SET : ATTRB_UNSET;
                        
                    break;
                }
            }
        }
    }
}

/* stretch_dialog:
  *  Stretch the dialog horizontally and vertically to the specified
  *   size and the font in use.
  *   (all the magic numbers come from the "historical" file selector)
  */
static void stretch_dialog(DIALOG *d, int32_t width, int32_t height, int32_t show_extlist)
{
    int32_t font_w, font_h, hpad, vpad;
    char tmp[16];
    
#ifdef HAVE_DIR_LIST
    
    /* horizontal settings */
    font_w = text_length(font, uconvert_ascii("A", tmp));
    
    if(width == 0)
        width = (int32_t)(0.95*screen->w);
        
    hpad = (int32_t)(0.05*width);
    
    d[FS_WIN].w     = width;
    d[FS_WIN].x     = 0;
    d[FS_EDIT].w    = d[FS_WIN].w - 2*hpad;
    d[FS_EDIT].x    = hpad;
    d[FS_CANCEL].w  = 10*font_w;
    d[FS_CANCEL].x  = d[FS_WIN].w - hpad - d[FS_CANCEL].w;
    d[FS_OK].w      = d[FS_CANCEL].w;
    d[FS_OK].x      = d[FS_CANCEL].x;
    d[FS_TYPES].x   = hpad;
    d[FS_TYPES].w   = d[FS_OK].x - 2*hpad;
    d[FS_FILES].w   = d[FS_TYPES].w;
    d[FS_FILES].x   = d[FS_TYPES].x;
    d[FS_EXPLORER].w = d[FS_CANCEL].w;
    d[FS_EXPLORER].x = d[FS_CANCEL].x;
    d[FS_DISKS].w   = d[FS_OK].w;
    d[FS_DISKS].x   = d[FS_OK].x;
    d[FS_YIELD].x   = 0;
    
    /* vertical settings */
    font_h = text_height(font);
    
    if(height == 0)
        height = (int32_t)(0.80*SCREEN_H);
        
    vpad = (int32_t)(0.05*height);
    
    d[FS_WIN].h     = height;
    d[FS_WIN].y     = 0;
    d[FS_EDIT].h    = font_h+6;
    d[FS_EDIT].y    = 2*vpad + font_h + 4;
    d[FS_CANCEL].h  = font_h + 13;
    d[FS_CANCEL].y  = d[FS_WIN].h - vpad - d[FS_CANCEL].h;
    d[FS_OK].h      = d[FS_CANCEL].h;
    d[FS_OK].y      = d[FS_CANCEL].y - vpad/2 - d[FS_OK].h;
    d[FS_TYPES].h   = show_extlist?font_h+8:0;
    d[FS_TYPES].y   = d[FS_WIN].h-d[FS_TYPES].h-vpad;
    d[FS_FILES].h   = ((((d[FS_WIN].h - 2*vpad - d[FS_EDIT].y - d[FS_EDIT].h - (show_extlist?(d[FS_TYPES].h+vpad):0))-8)/font_h)*font_h)+8;
    d[FS_FILES].y   = (show_extlist?d[FS_TYPES].y:d[FS_WIN].h)-vpad-d[FS_FILES].h;
    d[FS_EXPLORER].h = d[FS_OK].h;
    d[FS_EXPLORER].y = d[FS_OK].y - vpad/2 - d[FS_OK].h;
    d[FS_DISKS].y   = d[FS_FILES].y;
    d[FS_DISKS].h   = font_h+8;
    d[FS_YIELD].y   = 0;
    
#else
    
    /* horizontal settings */
    font_w = text_length(font, uconvert_ascii("A", tmp));
    
    if(width == 0)
        width = (int32_t)(0.95*screen->w);
    
    hpad = (int32_t)(0.05*width);
    
    d[FS_WIN].w     = width;
    d[FS_WIN].x     = 0;
    d[FS_EDIT].w    = d[FS_WIN].w - 2*hpad;
    d[FS_EDIT].x    = hpad;
    d[FS_OK].w      = 10*font_w;
    d[FS_OK].x      = (d[FS_WIN].w - 2*d[FS_OK].w - hpad + 1)/2;
    d[FS_CANCEL].w  = d[FS_OK].w;
    d[FS_CANCEL].x  = d[FS_WIN].w - d[FS_OK].x - d[FS_CANCEL].w;
    d[FS_TYPES].x   = hpad;
    d[FS_TYPES].w   = d[FS_WIN].w - 2*hpad;
    d[FS_FILES].x   = d[FS_TYPES].x;
    d[FS_FILES].w   = d[FS_TYPES].w;
    d[FS_EXPLORER].w = 45;
    d[FS_EXPLORER].x = width - 50;
    d[FS_YIELD].x   = 0;
    
    /* vertical settings */
    font_h = text_height(font);
    
    if(height == 0)
        height = (int32_t)(0.95*SCREEN_H);
    
    vpad = (int32_t)(0.04*height);
    
    d[FS_WIN].h     = height;
    d[FS_WIN].y     = 0;
    d[FS_EDIT].h    = font_h+6;
    d[FS_EDIT].y    = 2*vpad + font_h + 4;
    d[FS_OK].h      = font_h + 13;
    d[FS_OK].y      = d[FS_WIN].h - vpad - d[FS_OK].h;
    d[FS_CANCEL].h  = d[FS_OK].h;
    d[FS_CANCEL].y  = d[FS_OK].y;
    d[FS_TYPES].h   = show_extlist?font_h+8:0;
    d[FS_TYPES].y   = d[FS_OK].y-d[FS_TYPES].h-vpad;
    d[FS_FILES].h   = ((((d[FS_OK].y - 2*vpad - d[FS_EDIT].y - d[FS_EDIT].h - (show_extlist?(d[FS_TYPES].h+vpad):0))-8)/font_h)*font_h)+8;
    d[FS_FILES].y   = (show_extlist?d[FS_TYPES].y:d[FS_OK].y)-vpad-d[FS_FILES].h;
    d[FS_EXPLORER].h = d[FS_OK].h;
    d[FS_EXPLORER].y = d[FS_OK].y;
    d[FS_YIELD].y   = 0;
#endif
}

/* enlarge_file_selector:
 * Enlarges the dialog for Large Mode. -L
 */
void enlarge_file_selector(int32_t width, int32_t height)
{
    if(file_selector[0].d1==0)
    {
        stretch_dialog(file_selector, width, height, 1);
    }
    
    bool show_extlist = file_selector[FS_TYPES].proc != fs_dummy_proc;
    
	large_dialog(file_selector);
	int32_t bottom =
#ifndef HAVE_DIR_LIST
		file_selector[FS_OK].y;
#else
		file_selector[FS_WIN].y+file_selector[FS_WIN].h-8;
#endif
	file_selector[FS_FILES].dp2=NULL;
	file_selector[FS_TYPES].y = bottom-file_selector[FS_TYPES].h-5;
	file_selector[FS_FILES].h=show_extlist ? 152 : 168;
	file_selector[FS_FILES].y = (show_extlist ? file_selector[FS_TYPES].y:bottom)-(file_selector[FS_FILES].h+5);
	file_selector[FS_EDIT].y = file_selector[FS_FILES].y-26;
	((ListData *)file_selector[FS_FILES].dp)->font = &a4fonts[font_lfont_l];
	file_selector[FS_TYPES].dp2=NULL;
	file_selector[FS_TYPES].h=20;
	((ListData *)file_selector[FS_TYPES].dp)->font = &a4fonts[font_lfont_l];
#ifdef HAVE_DIR_LIST
	file_selector[FS_DISKS].dp2=NULL;
	file_selector[FS_DISKS].h=20;
	((ListData *)file_selector[FS_DISKS].dp)->font = &a4fonts[font_lfont_l];
#endif
	
	#define DIFF_VAL 30
	if(file_selector[FS_EDIT].y - DIFF_VAL < file_selector[FS_WIN].y)
	{
		int32_t diff = file_selector[FS_WIN].y - (file_selector[FS_EDIT].y - DIFF_VAL);
		file_selector[FS_WIN].y -= diff;
		file_selector[FS_WIN].h += diff;
	}
    jwin_center_dialog(file_selector);
}

/* jwin_file_select_ex:
  *  NOTE: do not call directly, see base/files.h
  *
  *  Displays the JWin file selector, with the message as caption.
  *  Allows the user to select a file, and stores the selection in the
  *  path buffer, whose length in bytes is given by size and should have
  *  room for at least 80 characters. The files are filtered according to
  *  the file extensions in ext. Passing NULL includes all files, "PCX;BMP"
  *  includes only files with .PCX or .BMP extensions. Returns zero if it
  *  was closed with the Cancel button or non-zero if it was OK'd.
  */
int32_t jwin_file_select_ex(AL_CONST char *message, char *path, AL_CONST char *ext, int32_t size, int32_t width, int32_t height, FONT *title_font)
{
    if (ext && ext[0] == '.' && strlen(ext) > 1)
        ext = &ext[1];

    static attrb_state_t default_attrb_state[ATTRB_MAX] = DEFAULT_ATTRB_STATE;
    int32_t ret;
    char *p;
    char tmp[32];
    ASSERT(message);
    ASSERT(path);
    
    if(title_font)
    {
        file_selector[0].dp2=title_font;
    }
    
    if(width == OLD_FILESEL_WIDTH)
        width = 304;
        
#ifdef HAVE_DIR_LIST
        
    if(height == OLD_FILESEL_HEIGHT)
        height = 160;
        
#else
        
    if(height == OLD_FILESEL_HEIGHT)
        height = 188;
        
#endif
        
    /* for fs_dlist_proc() */
    ASSERT(size >= 4 * uwidth_max(U_CURRENT));
    
    usetc(updir, 0);
    file_selector[FS_WIN].dp = (char *)message;
    file_selector[FS_EDIT].d1 = size/uwidth_max(U_CURRENT) - 1;
    file_selector[FS_EDIT].dp = path;
    file_selector[FS_OK].dp = (void*)get_config_text("OK");
    file_selector[FS_CANCEL].dp = (void*)get_config_text("Cancel");
    file_selector[FS_EXPLORER].dp = (void*)EXPLORER_LABEL;
#ifndef HAS_NFD
    file_selector[FS_EXPLORER].flags |= D_HIDDEN;
#endif
    
    /* Set default attributes. */
    memcpy(attrb_state, default_attrb_state, sizeof(default_attrb_state));
    
    /* Parse extension string. */
//  if (ext)// && ugetc(ext))
    {
        parse_extension_string(ext);
    }
    
    if(!ugetc(path))
    {
        get_root_path(path, size);
    }
    
    clear_keybuf();
    
    do
    {
        rest(1);
    }
    while(gui_mouse_b());
    
    file_selector[FS_TYPES].proc = fs_dummy_proc;
	// Z_message("Calling enlarge_file_selector(%d,%d)\n", width, height);
    enlarge_file_selector(width, height);
    ret = do_zqdialog(file_selector, FS_EDIT);
    
    if(fext)
    {
        free(fext);
        fext = NULL;
    }
    
    if(fext_p)
    {
        _al_free(fext_p);
        fext_p = NULL;
    }

    if (ret == FS_EXPLORER)
        return ret;

    if((ret == FS_CANCEL) || (ret == FS_WIN) || (!ugetc(get_filename(path))))
        return FALSE;
        
    p = get_extension(path);
    
    if((!ugetc(p)) && (ext) && (!ustrpbrk(ext, uconvert_ascii(" ,;", tmp))))
    {
        size -= ((int32_t)(size_t)p - (int32_t)(size_t)path + ucwidth('.'));
        
        if(size >= uwidth_max(U_CURRENT) + ucwidth(0))          /* do not end with '.' */
        {
            p += usetc(p, '.');
            ustrzcpy(p, size, ext);
        }
    }
    
    return TRUE;
}

static int32_t count_ext_list()
{
    int32_t c = 0;
    
    if(fext_list)
    {
        while(fext_list[c].text)
            c++;
    }
    
    return c;
}

/* fs_elist_getter:
  *  Listbox data getter routine for the file selector disk list.
  */
static const char *fs_elist_getter(int32_t index, int32_t *list_size)
{
    if(index < 0)
    {
        if(list_size)
            *list_size = count_ext_list();
            
        return NULL;
    }
    
    return fext_list[index].text;
}

/* fs_elist_proc:
  *  Dialog procedure for the file selector disk list.
  */
static int32_t fs_elist_proc(int32_t msg, DIALOG *d, int32_t c)
{
    int32_t ret;
    int32_t sel = d->d1;
    char *s, *tok;
    char tmp[80], ext[80];
    static char ext_tokens[] = " ,;";
    
    s = (char *) file_selector[FS_EDIT].dp;
    
    if(msg == MSG_START)
    {
        d->d2 = 0;
    }
    
    ret = jwin_droplist_proc(msg, d, c);
    
    if((sel != d->d1) || (ret & D_CLOSE))
    {
        // change the extension(s)
        const char* ext2 = fext_list[d->d1].ext == NULL ? "" : fext_list[d->d1].ext;
        fext = (char*) malloc(strlen(ext2) + 1);
        strcpy(fext, ext2);
//    if (fext)// && ugetc(fext))
        {
            parse_extension_string(fext);
        }
        
        // check whether the extension on the current file name is still valid
        if((fext) && (strlen(get_filename(s))))
        {
            strcpy(tmp, fext);
            strcpy(ext, get_extension(s));
            tok = strtok(tmp, ext_tokens);
            
            while(tok)
            {
                if(stricmp(ext, tok) == 0)
                    break;
                    
                tok = strtok(NULL, ext_tokens);
            }
            
            // not valid, replace file name
            if(!tok)
                replace_filename(s, flist->dir, "", 256);
        }
        
        SEND_MESSAGE(file_selector+FS_FILES, MSG_START, 0);
        SEND_MESSAGE(file_selector+FS_FILES, MSG_DRAW, 0);
        SEND_MESSAGE(file_selector+FS_EDIT, MSG_START, 0);
        SEND_MESSAGE(file_selector+FS_EDIT, MSG_DRAW, 0);
        
        if(ret & D_CLOSE)
            return (ret | SEND_MESSAGE(file_selector+FS_EDIT, MSG_KEY, 0)) & ~D_CLOSE;
    }
    
    return ret;
}

/* jwin_dfile_select_ex:
  *  NOTE: do not call directly, see base/files.h
  *
  *  Same as jwin_file_select_ex except that it returns TRUE on OK, even
  *  if the selected path doesn't include a file name.
  *  This is for getting directories instead of files, so a blank filename isn't an issue.
  */
int32_t jwin_dfile_select_ex(AL_CONST char *message, char *path, AL_CONST char *ext, int32_t size, int32_t width, int32_t height, FONT *title_font)
{
    static attrb_state_t default_attrb_state[ATTRB_MAX] = DEFAULT_ATTRB_STATE;
    int32_t ret;
    char *p;
    char tmp[32];
    ASSERT(message);
    ASSERT(path);
    
    if(title_font)
    {
        file_selector[0].dp2=title_font;
    }
    
    if(width == OLD_FILESEL_WIDTH)
        width = 304;
        
#ifdef HAVE_DIR_LIST
        
    if(height == OLD_FILESEL_HEIGHT)
        height = 160;
        
#else
        
    if(height == OLD_FILESEL_HEIGHT)
        height = 188;
        
#endif
        
    /* for fs_dlist_proc() */
    ASSERT(size >= 4 * uwidth_max(U_CURRENT));
    
    usetc(updir, 0);
    file_selector[FS_WIN].dp = (char *)message;
    file_selector[FS_EDIT].d1 = size/uwidth_max(U_CURRENT) - 1;
    file_selector[FS_EDIT].dp = path;
    file_selector[FS_OK].dp = (void*)get_config_text("OK");
    file_selector[FS_CANCEL].dp = (void*)get_config_text("Cancel");
    file_selector[FS_EXPLORER].dp = (void*)EXPLORER_LABEL;
#ifndef HAS_NFD
    file_selector[FS_EXPLORER].flags |= D_HIDDEN;
#endif
    
    /* Set default attributes. */
    memcpy(attrb_state, default_attrb_state, sizeof(default_attrb_state));
    
    /* Parse extension string. */
//  if (ext)// && ugetc(ext))
    {
        parse_extension_string(ext);
    }
    
    if(!ugetc(path))
    {
        get_root_path(path, size);
    }
    
    clear_keybuf();
    
    do
    {
        rest(1);
    }
    while(gui_mouse_b());
    
    file_selector[FS_TYPES].proc = fs_dummy_proc;
    enlarge_file_selector(width, height);
    ret = do_zqdialog(file_selector, FS_EDIT);
    
    if(fext)
    {
        free(fext);
        fext = NULL;
    }
    
    if(fext_p)
    {
        _al_free(fext_p);
        fext_p = NULL;
    }

    if (ret == FS_EXPLORER)
        return ret;
    
    if((ret == FS_CANCEL) || (ret == FS_WIN))
        return FALSE;
        
    if(!ugetc(get_filename(path)))
        return TRUE;
        
    p = get_extension(path);
    
    if((!ugetc(p)) && (ext) && (!ustrpbrk(ext, uconvert_ascii(" ,;", tmp))))
    {
        size -= ((int32_t)(size_t)p - (int32_t)(size_t)path + ucwidth('.'));
        
        if(size >= uwidth_max(U_CURRENT) + ucwidth(0))          /* do not end with '.' */
        {
            p += usetc(p, '.');
            ustrzcpy(p, size, ext);
        }
    }
    
    return TRUE;
}

/* get_root_path:
  *  Gets the program root directory as a path
  */
void get_root_path(char* path, int32_t size)
{
#ifdef HAVE_DIR_LIST
	int32_t drive = _al_getdrive();
#else
	int32_t drive = 0;
#endif

    _al_getdcwd(drive, path, size - ucwidth(OTHER_PATH_SEPARATOR));
	fix_filename_case(path);
	fix_filename_slashes(path);
	put_backslash(path);
}

/* relativize_path:
  *  Takes a path, and returns its relative path from the root directory.
  */
void relativize_path(char* dest, char const* src_path)
{
	char rootpath[PATH_MAX] = {0};
	get_root_path(rootpath, PATH_MAX);
    strcpy(dest, std::filesystem::relative(src_path, rootpath).string().c_str());
}

/* relativize_path:
  *  Takes a path, and returns its relative path from the root directory.
  */
std::string relativize_path(std::string src_path)
{
	char rootpath[PATH_MAX] = {0};
	get_root_path(rootpath, PATH_MAX);
    return std::filesystem::relative(src_path, rootpath).string();
}

/* derelativize_path:
  *  Takes a relative path from the root directory, and returns its' absolute path.
  */
void derelativize_path(char* dest, char const* src_path)
{
    char rootpath[PATH_MAX] = {0};
	get_root_path(rootpath, PATH_MAX);
    auto result = std::filesystem::path(rootpath) / src_path;
    strcpy(dest, result.string().c_str());
}

/* derelativize_path:
  *  Takes a relative path from the root directory, and returns its' absolute path.
  */
std::string derelativize_path(std::string src_path)
{
	char rootpath[PATH_MAX] = {0};
	get_root_path(rootpath, PATH_MAX);
    return (std::filesystem::path(rootpath) / src_path).string();
}

/* jwin_file_browse_ex:
  *  NOTE: do not call directly, see base/files.h
  *
  *  Same as jwin_file_select but it lets you give it a list of
  *  possible extensions to choose from.
  */
int32_t jwin_file_browse_ex(AL_CONST char *message, char *path, EXT_LIST *list, int32_t *list_sel, int32_t size, int32_t width, int32_t height, FONT *title_font)
{
    static attrb_state_t default_attrb_state[ATTRB_MAX] = DEFAULT_ATTRB_STATE;
    int32_t ret;
    char *p;
    char tmp[32];
    ASSERT(message);
    ASSERT(path);
    
    if(title_font)
    {
        file_selector[0].dp2=title_font;
    }
    
    if(width == OLD_FILESEL_WIDTH)
        width = 304;
        
#ifdef HAVE_DIR_LIST
        
    if(height == OLD_FILESEL_HEIGHT)
        height = 160;
        
#else
        
    if(height == OLD_FILESEL_HEIGHT)
        height = 188;
        
#endif
        
    /* for fs_dlist_proc() */
    ASSERT(size >= 4 * uwidth_max(U_CURRENT));
    
    usetc(updir, 0);
    file_selector[FS_WIN].dp = (char *)message;
    file_selector[FS_EDIT].d1 = size/uwidth_max(U_CURRENT) - 1;
    file_selector[FS_EDIT].dp = path;
    file_selector[FS_OK].dp = (void*)get_config_text("OK");
    file_selector[FS_CANCEL].dp = (void*)get_config_text("Cancel");
    file_selector[FS_EXPLORER].dp = (void*)EXPLORER_LABEL;
#ifndef HAS_NFD
    file_selector[FS_EXPLORER].flags |= D_HIDDEN;
#endif

    fext_list = list;
    const char* ext2 = list[*list_sel].ext == NULL ? "" : list[*list_sel].ext;
    fext = (char*) malloc(strlen(ext2) + 1);
    strcpy(fext, ext2);
    file_selector[FS_TYPES].d1 = *list_sel;
    
    /* Set default attributes. */
    memcpy(attrb_state, default_attrb_state, sizeof(default_attrb_state));
    
    /* Parse extension string. */
//  if (fext)// && ugetc(fext))
    {
        parse_extension_string(fext);
    }
    
    if(!ugetc(path))
    {
        get_root_path(path, size);
    }
    
    clear_keybuf();
    
    do
    {
        rest(1);
    }
    while(gui_mouse_b());
    
    file_selector[FS_TYPES].proc = fs_elist_proc;
    enlarge_file_selector(width,height);
    ret = do_zqdialog(file_selector, FS_EDIT);
    
    if((ret == FS_EXPLORER) || (ret == FS_CANCEL) || (ret == FS_WIN) || (!ugetc(get_filename(path))))
    {
        if(fext)
        {
            free(fext);
            fext = NULL;
        }

        if(fext_p)
        {
            _al_free(fext_p);
            fext_p = NULL;
        }

        if (ret == FS_EXPLORER)
            return ret;
        return FALSE;
    }
        
    *list_sel = file_selector[FS_TYPES].d1;
    
    p = get_extension(path);
    
    if((!ugetc(p)) && (fext) && (!ustrpbrk(fext, uconvert_ascii(" ,;", tmp))))
    {
        size -= ((int32_t)(size_t)p - (int32_t)(size_t)path + ucwidth('.'));
        
        if(size >= uwidth_max(U_CURRENT) + ucwidth(0))          /* do not end with '.' */
        {
            p += usetc(p, '.');
            ustrzcpy(p, size, fext);
        }
    }

    if(fext)
    {
        free(fext);
        fext = NULL;
    }

    if(fext_p)
    {
        _al_free(fext_p);
        fext_p = NULL;
    }
    
    return TRUE;
}


void FLIST::load(const char* path)
{
	char tmp[32];
	for(int32_t i=0; i<size; i++)
		if(name[i])
			free(name[i]);

	size = 0;

	replace_filename(dir, path, uconvert_ascii("*.*", tmp), sizeof(dir));
	
	FLIST* tmplist = flist;
	flist = this;

	/* The semantics of the attributes passed to file_select_ex() is
	  * different from that of for_each_file_ex() in one case: when
	  * the 'd' attribute is not mentioned in the set of characters,
	  * the other attributes are not taken into account for directories,
	  * i.e the directories are all included. So we can't filter with
	  * for_each_file_ex() in that case.
	  */
	if(attrb_state[ATTRB_DIREC] == ATTRB_ABSENT)
		/* accept all dirs */
		for_each_file_ex(dir, 0 , FA_LABEL, fs_flist_putter, (void *)1UL /* check */);
	else
		/* don't check */
		for_each_file_ex(dir, build_attrb_flag(ATTRB_SET), build_attrb_flag(ATTRB_UNSET) | FA_LABEL, fs_flist_putter, (void *)0UL);
	
	flist = tmplist;
	
	usetc(get_filename(dir), 0);
}

bool FLIST::get(int32_t index, char* buf)
{
	if(index < 0 || index >= size)
		return false;
	strcpy(buf, name[index]);
	return true;
}

void FLIST::clear()
{
	for(int32_t i=0; i<size; i++)
		if(name[i])
			free(name[i]);
	
	size = 0;
}
