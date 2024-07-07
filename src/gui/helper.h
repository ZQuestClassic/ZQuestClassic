#ifndef ZC_GUI_HELPER_H_
#define ZC_GUI_HELPER_H_

// This is to make static_assert failures stand out, since they can easily
// get lost in all the other output errors produce with metaprogramming.
// Compiler differences complicate this slightly.
// G++ interprets \n as a line break.
// Clang outputs "\n".
// MSVC outputs line breaks, but only displays the first line.
#if defined(_MSC_VER) || defined(__clang__)
    #define ZCGUI_STATIC_ASSERT(cond, msg) \
        static_assert(cond, "*** GUI ERROR *** " msg)
    #define ZCGUI_NEWLINE " "
#else
    #define ZCGUI_STATIC_ASSERT(cond, msg)                 \
        static_assert(cond,                                \
            "\n====================================GUI "   \
            "ERROR===================================\n\n" \
            msg                                            \
            "\n\n======================================="  \
            "=========================================\n")
    #define ZCGUI_NEWLINE "\n"
#endif

#define ZCGUI_STRINGIZE(arg) ZCGUI_STRINGIZE_2(arg)
#define ZCGUI_STRINGIZE_2(arg) #arg

#define ZCGUI_WIDGET_NAME(baseName) baseName
#define ZCGUI_WIDGET_STR(baseName) ZCGUI_STRINGIZE(ZCGUI_WIDGET_NAME(baseName))
#define ZCGUI_PROP_NAME(baseName) baseName
#define ZCGUI_PROP_STR(baseName) ZCGUI_STRINGIZE(ZCGUI_PROP_NAME(baseName))

#endif
