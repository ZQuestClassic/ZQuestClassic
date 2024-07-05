#ifndef ZC_GUI_MACROS_H_
#define ZC_GUI_MACROS_H_

#include <type_traits>
#include <utility>
#include "gui/helper.h"

namespace GUI::Internal
{

template<typename parent, typename child>
inline void allowChild(const std::shared_ptr<parent>&, const std::shared_ptr<child>&)
{
    #define ZCGUI_REQUIRE_PARENT(p, c, msg) \
        ZCGUI_STATIC_ASSERT((std::is_same_v<parent, p> || !std::is_same_v<child, c>), msg)
    #define ZCGUI_REQUIRE_CHILD(p, c, msg) \
        ZCGUI_STATIC_ASSERT((!std::is_same_v<parent, p> || std::is_same_v<child, c>), msg)

	ZCGUI_REQUIRE_PARENT(::GUI::TabPanel, ::GUI::TabRef, "Tabrefs can only be added to TabPanels.");
	ZCGUI_REQUIRE_CHILD(::GUI::TabPanel, ::GUI::TabRef, "Only Tabrefs can be added to TabPanels.");
}

} // namespace GUI::Internal


#define ZCGUI_DECLARE_PROPERTY(name)                                                               \
struct name##Prop                                                                                  \
{                                                                                                  \
    struct TagType {};                                                                             \
                                                                                                   \
    template<typename T>                                                                           \
    struct Value: ::GUI::Props::Property                                                           \
    {                                                                                              \
        using TagType = name##Prop::TagType;                                                       \
        static constexpr TagType tag = {};                                                         \
        T&& val;                                                                                   \
                                                                                                   \
        inline constexpr Value(T&& t): val(std::forward<T>(t))                                     \
        {}                                                                                         \
                                                                                                   \
        inline constexpr T&& get() const                                                           \
        {                                                                                          \
            return std::forward<T>(val);                                                           \
        }                                                                                          \
                                                                                                   \
        /* This is called to ensure the same property isn't given twice for one widget. */         \
        template<typename PropsSoFar>                                                              \
        void assertUnique(PropsSoFar) const                                                        \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT((!std::is_base_of_v<TagType, PropsSoFar>),                         \
                "Property " ZCGUI_PROP_STR(name) " provided more than once.");                     \
        }                                                                                          \
                                                                                                   \
        /* This is called when used on a widget that doesn't accept  this property. */             \
        template<bool b = false>                                                                   \
        void assertInvalid() const                                                                 \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT(b,                                                                 \
                "Property '" ZCGUI_PROP_STR(name) "' is not valid for this widget");               \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    template<typename T>                                                                           \
	inline constexpr Value<T> operator=(T&& t) const                                               \
    {                                                                                              \
        return Value<T>(std::forward<T>(t));                                                       \
    }                                                                                              \
};                                                                                                 \
                                                                                                   \
static constexpr name##Prop ZCGUI_PROP_NAME(name);


// For those irritating times when C++ isn't smart enough to figure out
// what type you want it to take...
// It should still be possible to use the property for other types.
// This just sets a default.
#define ZCGUI_DECLARE_PROPERTY_AS(name, type)                                                      \
struct name##Prop                                                                                  \
{                                                                                                  \
    struct TagType {};                                                                             \
                                                                                                   \
    template<typename T>                                                                           \
    struct Value: ::GUI::Props::Property                                                           \
    {                                                                                              \
        using TagType = name##Prop::TagType;                                                       \
        static constexpr TagType tag = {};                                                         \
        T&& val;                                                                                   \
                                                                                                   \
        inline constexpr Value(T&& t): val(std::forward<T>(t))                                     \
        {}                                                                                         \
                                                                                                   \
        inline constexpr T&& get() const                                                           \
        {                                                                                          \
            return std::forward<T>(val);                                                           \
        }                                                                                          \
                                                                                                   \
        /* This is called to ensure the same property isn't given twice for one widget. */         \
        template<typename PropsSoFar>                                                              \
        void assertUnique(PropsSoFar) const                                                        \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT((!std::is_base_of_v<TagType, PropsSoFar>),                         \
                "Property " ZCGUI_PROP_STR(name) " provided more than once.");                     \
        }                                                                                          \
                                                                                                   \
        /* This is called when used on a widget that doesn't accept  this property. */             \
        template<bool b = false>                                                                   \
        void assertInvalid() const                                                                 \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT(b,                                                                 \
                "Property '" ZCGUI_PROP_STR(name) "' is not valid for this widget");               \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    template<typename T = type>                                                                    \
	inline constexpr Value<T> operator=(T&& t) const                                               \
    {                                                                                              \
        return Value<T>(std::forward<T>(t));                                                       \
    }                                                                                              \
};                                                                                                 \
                                                                                                   \
static constexpr name##Prop ZCGUI_PROP_NAME(name);


// Builder for most widgets. Accepts all common properties.
#define ZCGUI_BUILDER_START(widgetType)                                                            \
struct widgetType##Builder                                                                         \
{                                                                                                  \
    struct Dummy;                                                                                  \
                                                                                                   \
    std::shared_ptr<::GUI:: widgetType> ptr;                                                       \
                                                                                                   \
    inline widgetType##Builder(std::shared_ptr<::GUI:: widgetType>&& p):                           \
        ptr(p)                                                                                     \
    {}                                                                                             \
                                                                                                   \
    inline std::shared_ptr<::GUI:: widgetType> resolve() const                                     \
    {                                                                                              \
        return ptr;                                                                                \
    }                                                                                              \
                                                                                                   \
    /* This function will be called if a property not valid for this widget is given. */           \
    template<typename PropType, typename PropTagType>                                              \
    void applyProp(PropType&& prop, PropTagType&&)                                                 \
    {                                                                                              \
        prop.assertInvalid();                                                                      \
    }                                                                                              \
                                                                                                   \
    /* Accept properties common to most widgets. */                                                \
    ZCGUI_ACCEPT_PROP(width, overrideWidth, ::GUI::Size)                                           \
    ZCGUI_ACCEPT_PROP(height, overrideHeight, ::GUI::Size)                                         \
    ZCGUI_ACCEPT_PROP(maxwidth, capWidth, ::GUI::Size)                                             \
    ZCGUI_ACCEPT_PROP(maxheight, capHeight, ::GUI::Size)                                           \
    ZCGUI_ACCEPT_PROP(minwidth, minWidth, ::GUI::Size)                                             \
    ZCGUI_ACCEPT_PROP(minheight, minHeight, ::GUI::Size)                                           \
    ZCGUI_ACCEPT_PROP(visible, setVisible, bool)                                                   \
    ZCGUI_ACCEPT_PROP(hAlign, setHAlign, float)                                                    \
    ZCGUI_ACCEPT_PROP(vAlign, setVAlign, float)                                                    \
    ZCGUI_ACCEPT_PROP(leftMargin, setLeftMargin, ::GUI::Size)                                      \
    ZCGUI_ACCEPT_PROP(rightMargin, setRightMargin, ::GUI::Size)                                    \
    ZCGUI_ACCEPT_PROP(topMargin, setTopMargin, ::GUI::Size)                                        \
    ZCGUI_ACCEPT_PROP(bottomMargin, setBottomMargin, ::GUI::Size)                                  \
    ZCGUI_ACCEPT_PROP(hMargins, setHMargins, ::GUI::Size)                                          \
    ZCGUI_ACCEPT_PROP(vMargins, setVMargins, ::GUI::Size)                                          \
    ZCGUI_ACCEPT_PROP(margins, setMargins, ::GUI::Size)                                            \
    ZCGUI_ACCEPT_PROP(leftPadding, setLeftPadding, ::GUI::Size)                                    \
    ZCGUI_ACCEPT_PROP(rightPadding, setRightPadding, ::GUI::Size)                                  \
    ZCGUI_ACCEPT_PROP(topPadding, setTopPadding, ::GUI::Size)                                      \
    ZCGUI_ACCEPT_PROP(bottomPadding, setBottomPadding, ::GUI::Size)                                \
    ZCGUI_ACCEPT_PROP(hPadding, setHPadding, ::GUI::Size)                                          \
    ZCGUI_ACCEPT_PROP(vPadding, setVPadding, ::GUI::Size)                                          \
    ZCGUI_ACCEPT_PROP(padding, setPadding, ::GUI::Size)                                            \
    ZCGUI_ACCEPT_PROP(focused, setFocused, bool)                                                   \
    ZCGUI_ACCEPT_PROP(framed, setFramed, bool)                                                     \
    ZCGUI_ACCEPT_PROP(nopad, setNoPad, bool)                                                     \
    ZCGUI_ACCEPT_PROP(fitParent, setFitParent, bool)                                               \
    ZCGUI_ACCEPT_PROP(forceFitW, setForceFitWid, bool)                                             \
    ZCGUI_ACCEPT_PROP(forceFitH, setForceFitHei, bool)                                             \
    ZCGUI_ACCEPT_PROP(disabled, setDisabled, bool)                                                 \
    ZCGUI_ACCEPT_PROP(useFont, setFont, FONT*)                                                     \
    ZCGUI_ACCEPT_PROP(frameText, setFrameText, std::string const&)                                 \
    ZCGUI_ACCEPT_PROP(rowSpan, setRowSpan, uint8_t)                                                \
    ZCGUI_ACCEPT_PROP(colSpan, setColSpan, uint8_t)                                                \
    ZCGUI_ACCEPT_PROP(read_only, setReadOnly, bool)                                                \
    ZCGUI_ACCEPT_PROP(userData, setUserData, anything)

// Builder for widgets that don't accept common properties.
#define ZCGUI_MINIMAL_BUILDER_START(widgetType)                                                    \
struct widgetType##Builder                                                                         \
{                                                                                                  \
    struct Dummy;                                                                                  \
                                                                                                   \
    std::shared_ptr<::GUI:: widgetType> ptr;                                                       \
                                                                                                   \
    inline widgetType##Builder(std::shared_ptr<::GUI:: widgetType>&& p):                           \
        ptr(p)                                                                                     \
    {}                                                                                             \
                                                                                                   \
    inline std::shared_ptr<::GUI:: widgetType> resolve() const                                     \
    {                                                                                              \
        return ptr;                                                                                \
    }                                                                                              \
                                                                                                   \
    /* This function will be called if a property not valid for this widget is given. */           \
    template<typename PropType, typename PropTagType>                                              \
    void applyProp(PropType&& prop, PropTagType&&)                                                 \
    {                                                                                              \
        prop.assertInvalid();                                                                      \
    }


#define ZCGUI_ACCEPT_PROP(propName, function, ...)                                                 \
    /* Overrides the default for accepted properties. */                                           \
    template<typename PropType>                                                                    \
    inline void applyProp(PropType&& prop, ::GUI::Props::propName##Prop::TagType)                  \
    {                                                                                              \
        apply##propName(std::forward<PropType>(prop));                                             \
    }                                                                                              \
                                                                                                   \
    /* This one's called if the property is accepted but the type is incorrect. */                 \
    template<bool b = false>                                                                       \
    void apply##propName(...)                                                                      \
    {                                                                                              \
        ZCGUI_STATIC_ASSERT(b,                                                                     \
            "Invalid type for property '" #propName "'." ZCGUI_NEWLINE                             \
            "Type should be " #__VA_ARGS__ " (or convertible).");                                  \
    }                                                                                              \
                                                                                                   \
    /* And this one actually applies the property. */                                              \
    template<typename PropType>                                                                    \
    inline auto apply##propName(PropType&& prop) -> decltype(ptr->function(prop.get()))            \
    {                                                                                              \
        /* Presumably void, but just in case... */                                                 \
        return ptr->function(prop.get());                                                          \
    }


#define ZCGUI_SUGGEST_PROP(propName, suggestion)                                                   \
    /* Overrides the default... */                                                                 \
    template<typename PropType>                                                                    \
    inline void applyProp(PropType&& prop, ::GUI::Props::propName##Prop::TagType)                  \
    {                                                                                              \
        apply##propName(std::forward<PropType>(prop));                                             \
    }                                                                                              \
                                                                                                   \
    /* Fails and suggests an alternative. */                                                       \
    template<bool b = false>                                                                       \
    void apply##propName(...)                                                                      \
    {                                                                                              \
        ZCGUI_STATIC_ASSERT(b,                                                                     \
            "Property '" ZCGUI_PROP_STR(propName) "' is not valid for this widget." ZCGUI_NEWLINE  \
            "Did you mean '" ZCGUI_PROP_STR(suggestion)"'?");                                      \
    }                                                                                              \


#define ZCGUI_ACCEPT_ONE_CHILD(function)                                                           \
    template<int32_t counter = 1, typename ChildType, typename... MoreChildrenType>                    \
    inline void addChildren(ChildType&&, MoreChildrenType&&... moreChildren)                       \
    {                                                                                              \
        using DecayType = typename std::decay_t<ChildType>;                                        \
        constexpr bool isProp = std::is_base_of_v<::GUI::Props::Property, DecayType>;              \
        constexpr bool isWidget =                                                                  \
		    std::is_convertible_v<ChildType, std::shared_ptr<::GUI::Widget>>;                      \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        addChildren<counter+1>(std::forward<MoreChildrenType>(moreChildren)...);                   \
    }                                                                                              \
                                                                                                   \
    template<int32_t counter = 1, typename ChildType>                                                  \
    inline void addChildren(ChildType&& child)                                                     \
    {                                                                                              \
        using DecayType = typename std::decay_t<ChildType>;                                        \
        constexpr bool isProp = std::is_base_of_v<::GUI::Props::Property, DecayType>;              \
        constexpr bool isWidget =                                                                  \
		    std::is_convertible_v<ChildType, std::shared_ptr<::GUI::Widget>>;                      \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        ZCGUI_STATIC_ASSERT(counter == 1, "This widget can only have one child.");                 \
        Internal::allowChild(ptr, child);                                                          \
        ptr->function(child);                                                                      \
    }


#define ZCGUI_ACCEPT_MULTIPLE_CHILDREN(function)                                                   \
    template<typename ChildType, typename... MoreChildrenType>                                     \
    inline void addChildren(ChildType&& child, MoreChildrenType&&... moreChildren)                 \
    {                                                                                              \
        using DecayType = typename std::decay_t<ChildType>;                                        \
        constexpr bool isProp = std::is_base_of_v<::GUI::Props::Property, DecayType>;              \
        constexpr bool isWidget =                                                                  \
		    std::is_convertible_v<ChildType, std::shared_ptr<::GUI:: Widget>>;                     \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        Internal::allowChild(ptr, child);                                                          \
        ptr->function(child);                                                                      \
        addChildren(std::forward<MoreChildrenType>(moreChildren)...);                              \
    }                                                                                              \
                                                                                                   \
    template<typename ChildType>                                                                   \
    inline void addChildren(ChildType&& child)                                                     \
    {                                                                                              \
        using DecayType = typename std::decay_t<ChildType>;                                        \
        constexpr bool isProp = std::is_base_of_v<::GUI::Props::Property, DecayType>;              \
        constexpr bool isWidget =                                                                  \
		    std::is_convertible_v<ChildType, std::shared_ptr<::GUI::Widget>>;                      \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        Internal::allowChild(ptr, child);                                                          \
        ptr->function(child);                                                                      \
    }


// Reject children by default.
#define ZCGUI_BUILDER_END()                                                                        \
    template<bool b = false>                                                                       \
    void addChildren(...)                                                                          \
    {                                                                                              \
        ZCGUI_STATIC_ASSERT(b, "This widget cannot have children.");                               \
    }                                                                                              \
};


#define ZCGUI_BUILDER_FUNCTION(widgetType, functionName, implCreator)                              \
template<typename... PropsType>                                                                    \
inline std::shared_ptr<::GUI:: widgetType> ZCGUI_WIDGET_NAME(functionName)(PropsType&&... props)   \
{                                                                                                  \
    widgetType##Builder ret(::GUI::Internal:: implCreator());                                      \
    ::GUI::Internal::applyArgs(::GUI::Internal::dummy, ret,                                        \
        std::forward<PropsType>(props)...);                                                        \
    return ret.resolve();                                                                          \
}


#define ZCGUI_BUILDER_FUNCTION_TEMPLATE(widgetType, functionName, implCreator, templateParamType)  \
template<templateParamType TemplateParam, typename... PropsType>                                   \
inline std::shared_ptr<::GUI:: widgetType> ZCGUI_WIDGET_NAME(functionName)(PropsType&&... props)   \
{                                                                                                  \
    widgetType##Builder ret(::GUI::Internal:: implCreator(TemplateParam));                         \
    ::GUI::Internal::applyArgs(::GUI::Internal::dummy, ret,                                        \
        std::forward<PropsType>(props)...);                                                        \
    return ret.resolve();                                                                          \
}

#define ZCGUI_BUILDER_FUNCTION_TEMPLATE2(widgetType, functionName, implCreator, T, T2)  \
template<T t, T2 t2, typename... PropsType>                                   \
inline std::shared_ptr<::GUI:: widgetType> ZCGUI_WIDGET_NAME(functionName)(PropsType&&... props)   \
{                                                                                                  \
    widgetType##Builder ret(::GUI::Internal:: implCreator(t,t2));                         \
    ::GUI::Internal::applyArgs(::GUI::Internal::dummy, ret,                                        \
        std::forward<PropsType>(props)...);                                                        \
    return ret.resolve();                                                                          \
}

#endif
