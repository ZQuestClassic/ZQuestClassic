#ifndef ZC_GUI_HELPER_H
#define ZC_GUI_HELPER_H

#define ZCGUI_STATIC_ASSERT(cond, msg)         \
static_assert(cond,                            \
"\n====================================GUI "   \
"ERROR===================================\n\n" \
msg                                            \
"\n\n======================================="  \
"=========================================\n")

#define ZCGUI_STRINGIZE(arg) ZCGUI_STRINGIZE_2(arg)
#define ZCGUI_STRINGIZE_2(arg) #arg

#define ZCGUI_WIDGET_NAME(baseName) baseName
#define ZCGUI_WIDGET_STR(baseName) ZCGUI_STRINGIZE(ZCGUI_WIDGET_NAME(baseName))
#define ZCGUI_PROP_NAME(baseName) baseName
#define ZCGUI_PROP_STR(baseName) ZCGUI_STRINGIZE(ZCGUI_PROP_NAME(baseName))

#endif
