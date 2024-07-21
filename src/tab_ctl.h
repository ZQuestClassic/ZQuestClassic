#ifndef TAB_INTERN_H_
#define TAB_INTERN_H_

#include "base/zc_alleg.h"


#ifdef __cplusplus
extern "C"
{
#endif
int32_t vc2(int32_t x);
/* a tab panel */
typedef struct TABPANEL
{
    char *text;                   /* menu item text */
    int32_t flags;                    /* flags about the menu state */
    int32_t *dialog;                  /* pointer to group of dialog objects that this tab contains */
    int32_t objects;                  /* count of number of objects this tab handles; auto-calculated */
    int32_t *xy;                      /* x and y coords of those dialog objects. */
} TABPANEL;

bool uses_tab_arrows(TABPANEL *panel, int32_t maxwidth);
int32_t tab_count(TABPANEL *panel);
int32_t last_visible_tab(TABPANEL *panel, int32_t first_tab, int32_t maxwidth);
int32_t d_tab_proc(int32_t msg, DIALOG *d, int32_t c);
#ifdef __cplusplus
}
#endif
#endif
