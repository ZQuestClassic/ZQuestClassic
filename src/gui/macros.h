#ifndef ZC_GUI_MACROS_H
#define ZC_GUI_MACROS_H

#include <type_traits>
#include <utility>
#include "helper.h"

namespace gui::internal
{

// TODO: Can't this be done better?
template<typename parent, typename child>
inline void allowChild(std::shared_ptr<parent>, std::shared_ptr<child>)
{
    #define ZCGUI_DISALLOW_PAIRING(p, c, msg) \
        ZCGUI_STATIC_ASSERT(!(std::is_same<parent, p>::value && std::is_same<child, c>::value), msg)
    #define ZCGUI_REQUIRE_PARENT(p, c, msg) \
        ZCGUI_STATIC_ASSERT((std::is_same<parent, p>::value || !std::is_same<child, c>::value), msg)
}

} // namespace gui::internal


#define ZCGUI_DECLARE_PROPERTY(name)                                                               \
struct name##Prop                                                                                  \
{                                                                                                  \
    struct TagType {};                                                                             \
                                                                                                   \
    template<typename T>                                                                           \
    struct Value: ::gui::props::Property                                                           \
    {                                                                                              \
        using TagType=name##Prop::TagType;                                                         \
        static TagType tag;                                                                        \
        T&& val;                                                                                   \
                                                                                                   \
        Value(T&& t):                                                                              \
            val(std::forward<T>(t))                                                                \
        {                                                                                          \
        }                                                                                          \
                                                                                                   \
        inline T&& get() const                                                                     \
        {                                                                                          \
            return std::forward<T>(val);                                                           \
        }                                                                                          \
                                                                                                   \
        /* This is called to ensure the same property isn't given twice for one widget. */         \
        template<typename PropsSoFar>                                                              \
        void assertUnique(PropsSoFar) const                                                        \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT((!std::is_base_of<TagType, PropsSoFar>::value),                    \
                "Property " ZCGUI_PROP_STR(name) " provided more than once.");                     \
        }                                                                                          \
                                                                                                   \
        /* This is called when used on a widget that doesn't accept  this property. */             \
        template<bool b=false>                                                                     \
        void assertInvalid() const                                                                 \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT(b,                                                                 \
                "Property '" ZCGUI_PROP_STR(name) "' is not valid for this widget");               \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    template<typename T> Value<T> operator=(T&& t) const                                           \
    {                                                                                              \
        return Value<T>(std::forward<T>(t));                                                       \
    }                                                                                              \
};                                                                                                 \
                                                                                                   \
constexpr name##Prop ZCGUI_PROP_NAME(name);


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
    struct Value: ::gui::props::Property                                                           \
    {                                                                                              \
        using TagType=name##Prop::TagType;                                                         \
        static TagType tag;                                                                        \
        T&& val;                                                                                   \
                                                                                                   \
        Value(T&& t):                                                                              \
            val(std::forward<T>(t))                                                                \
        {                                                                                          \
        }                                                                                          \
                                                                                                   \
        inline T&& get() const                                                                     \
        {                                                                                          \
            return std::forward<T>(val);                                                           \
        }                                                                                          \
                                                                                                   \
        /* This is called to ensure the same property isn't given twice for one widget. */         \
        template<typename PropsSoFar>                                                              \
        void assertUnique(PropsSoFar) const                                                        \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT((!std::is_base_of<TagType, PropsSoFar>::value),                    \
                "Property " ZCGUI_PROP_STR(name) " provided more than once.");                     \
        }                                                                                          \
                                                                                                   \
        /* This is called when used on a widget that doesn't accept  this property. */             \
        template<bool b=false>                                                                     \
        void assertInvalid() const                                                                 \
        {                                                                                          \
            ZCGUI_STATIC_ASSERT(b,                                                                 \
                "Property '" ZCGUI_PROP_STR(name) "' is not valid for this widget");               \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    template<typename T=type> Value<T> operator=(T&& t) const                                      \
    {                                                                                              \
        return Value<T>(std::forward<T>(t));                                                       \
    }                                                                                              \
};                                                                                                 \
                                                                                                   \
constexpr name##Prop ZCGUI_PROP_NAME(name);


#define ZCGUI_BUILDER_START(widgetType)                                                            \
struct widgetType##Builder                                                                         \
{                                                                                                  \
    struct Dummy;                                                                                  \
                                                                                                   \
    std::shared_ptr<widgetType> ptr;                                                               \
                                                                                                   \
    widgetType##Builder(std::shared_ptr<widgetType>&& p):                                          \
        ptr(std::move(p))                                                                          \
    {                                                                                              \
                                                                                                   \
    }                                                                                              \
                                                                                                   \
    inline std::shared_ptr<widgetType> resolve() const                                             \
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
    inline void applyProp(PropType&& prop, ::gui::props::propName##Prop::TagType)                  \
    {                                                                                              \
        apply##propName(std::forward<PropType>(prop));                                             \
    }                                                                                              \
                                                                                                   \
    /* This one's called if the property is accepted but the type is incorrect. */                 \
    template<bool b=false>                                                                         \
    void apply##propName(...)                                                                      \
    {                                                                                              \
        ZCGUI_STATIC_ASSERT(b,                                                                     \
            "Invalid type for property '" #propName "'.\n"                                         \
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
    inline void applyProp(PropType&& prop, ::gui::props::propName##Prop::TagType)                  \
    {                                                                                              \
        apply##propName(std::forward<PropType>(prop));                                             \
    }                                                                                              \
                                                                                                   \
    /* Fails and suggests an alternative. */                                                       \
    template<bool b=false>                                                                         \
    void apply##propName(...)                                                                      \
    {                                                                                              \
        ZCGUI_STATIC_ASSERT(b,                                                                     \
            "Property '" ZCGUI_PROP_STR(propName) "' is not valid for this widget.\n"              \
            "Did you mean '" ZCGUI_PROP_STR(suggestion)"'?");                                      \
    }                                                                                              \


#define ZCGUI_ACCEPT_ONE_CHILD(function)                                                           \
    template<int counter=1, typename ChildType, typename... MoreChildrenType>                      \
    inline void addChildren(ChildType&&, MoreChildrenType&&... moreChildren)                       \
    {                                                                                              \
        using DecayType=typename std::decay<ChildType>::type;                                      \
        constexpr bool isProp=std::is_base_of<::gui::props::Property, DecayType>::value;           \
        constexpr bool isWidget=std::is_convertible<ChildType, std::shared_ptr<Widget>>::value;    \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        addChildren<counter+1>(std::forward<MoreChildrenType>(moreChildren)...);                   \
    }                                                                                              \
                                                                                                   \
    template<int counter=1, typename ChildType>                                                    \
    inline void addChildren(ChildType&& child)                                                     \
    {                                                                                              \
        using DecayType=typename std::decay<ChildType>::type;                                      \
        constexpr bool isProp=std::is_base_of<::gui::props::Property, DecayType>::value;           \
        constexpr bool isWidget=std::is_convertible<ChildType, std::shared_ptr<Widget>>::value;    \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        ZCGUI_STATIC_ASSERT(counter==1, "This widget cannot have multiple children.");             \
        internal::allowChild(ptr, child);                                                          \
        ptr->function(child);                                                                      \
    }


#define ZCGUI_ACCEPT_MULTIPLE_CHILDREN(function)                                                   \
    template<typename ChildType, typename... MoreChildrenType>                                     \
    inline void addChildren(ChildType&& child, MoreChildrenType&&... moreChildren)                 \
    {                                                                                              \
        using DecayType=typename std::decay<ChildType>::type;                                      \
        constexpr bool isProp=std::is_base_of<::gui::props::Property, DecayType>::value;           \
        constexpr bool isWidget=std::is_convertible<ChildType, std::shared_ptr<Widget>>::value;    \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        internal::allowChild(ptr, child);                                                          \
        ptr->function(child);                                                                      \
        addChildren(std::forward<MoreChildrenType>(moreChildren)...);                              \
    }                                                                                              \
                                                                                                   \
    template<typename ChildType>                                                                   \
    inline void addChildren(ChildType&& child)                                                     \
    {                                                                                              \
        using DecayType=typename std::decay<ChildType>::type;                                      \
        constexpr bool isProp=std::is_base_of<::gui::props::Property, DecayType>::value;           \
        constexpr bool isWidget=std::is_convertible<ChildType, std::shared_ptr<Widget>>::value;    \
        ZCGUI_STATIC_ASSERT(!isProp || isWidget, "Properties must come before children.");         \
        ZCGUI_STATIC_ASSERT(isProp || isWidget, "Not a widget property or widget.");               \
        internal::allowChild(ptr, child);                                                          \
        ptr->function(child);                                                                      \
    }


// Accept common properties and reject children by default.
#define ZCGUI_BUILDER_END()                                                                        \
    template<bool b=false>                                                                         \
    void addChildren(...)                                                                          \
    {                                                                                              \
        ZCGUI_STATIC_ASSERT(b, "This widget cannot have children.");                               \
    }                                                                                              \
    ZCGUI_ACCEPT_PROP(hAlign, setHAlign, int)                                                      \
    ZCGUI_ACCEPT_PROP(vAlign, setVAlign, int)                                                      \
    ZCGUI_ACCEPT_PROP(hPadding, setHPadding, int)                                                  \
    ZCGUI_ACCEPT_PROP(vPadding, setVPadding, int)                                                  \
};


#define ZCGUI_BUILDER_FUNCTION(widgetType, functionName, implCreator)                              \
template<typename... PropsType>                                                                    \
inline std::shared_ptr<widgetType> ZCGUI_WIDGET_NAME(functionName)(PropsType&&... props)           \
{                                                                                                  \
    widgetType##Builder ret(implCreator());                                                        \
    ::gui::internal::applyArgs(::gui::internal::dummy, ret,                                        \
        std::forward<PropsType>(props)...);                                                        \
    return ret.resolve();                                                                          \
}


#define ZCGUI_BUILDER_FUNCTION_TEMPLATE(widgetType, functionName, implCreator, templateParamType)  \
template<templateParamType TemplateParam, typename... PropsType>                                   \
inline std::shared_ptr<widgetType> ZCGUI_WIDGET_NAME(functionName)(PropsType&&... props)           \
{                                                                                                  \
    widgetType##Builder ret(implCreator(TemplateParam));                                           \
    ::gui::internal::applyArgs(::gui::internal::dummy, ret,                                        \
        std::forward<PropsType>(props)...);                                                        \
    return ret.resolve();                                                                          \
}

namespace gui::props
{

struct Property {};

// gcc -Weffc++ will complain "'operator=' should return a reference to '*this'"
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

ZCGUI_DECLARE_PROPERTY(boxPlacement)
ZCGUI_DECLARE_PROPERTY(checked)
ZCGUI_DECLARE_PROPERTY(data)
ZCGUI_DECLARE_PROPERTY(hAlign)
ZCGUI_DECLARE_PROPERTY(hPadding)
ZCGUI_DECLARE_PROPERTY(maxLength)
ZCGUI_DECLARE_PROPERTY(onClick)
ZCGUI_DECLARE_PROPERTY(onClose)
ZCGUI_DECLARE_PROPERTY(onSelectionChanged)
ZCGUI_DECLARE_PROPERTY(onValueChanged)
ZCGUI_DECLARE_PROPERTY(onEnter)
ZCGUI_DECLARE_PROPERTY(selectedValue)
ZCGUI_DECLARE_PROPERTY_AS(shortcuts, std::initializer_list<KeyboardShortcut>)
ZCGUI_DECLARE_PROPERTY(text)
ZCGUI_DECLARE_PROPERTY(title)
ZCGUI_DECLARE_PROPERTY(type)
ZCGUI_DECLARE_PROPERTY(vAlign)
ZCGUI_DECLARE_PROPERTY(vPadding)

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

} // namespace gui::props

namespace gui::internal
{

// Clang will complain that definitions of tag types aren't available.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif

template<typename ArgsSoFar, typename BuilderType>
inline void applyArgs(ArgsSoFar, BuilderType&&)
{
}

template<typename ArgsSoFar, typename BuilderType, typename WidgetType,
    typename... MoreArgsType>
inline void applyArgs(ArgsSoFar, BuilderType&& builder, std::shared_ptr<WidgetType> child,
    MoreArgsType&&... moreArgs)
{
    builder.addChildren(child, std::forward<MoreArgsType>(moreArgs)...);
}

template<typename PropsSoFar, typename BuilderType, typename WidgetType>
inline void applyArgs(PropsSoFar, BuilderType&& builder, std::shared_ptr<WidgetType> child)
{
    builder.addChildren(child);
}

template<typename PropsSoFar, typename BuilderType, typename PropType>
inline void applyArgs(PropsSoFar psf, BuilderType&& builder, PropType&& prop)
{
    using DecayType=typename std::decay<PropType>::type;
    ZCGUI_STATIC_ASSERT((std::is_base_of<props::Property, DecayType>::value),
        "Arguments must be widget properties or widgets.");
    prop.assertUnique(psf);

    builder.applyProp(std::forward<PropType>(prop), PropType::tag);
}

template<typename PropsSoFar, typename BuilderType, typename PropType, typename... MoreArgsType>
inline void applyArgs(PropsSoFar psf, BuilderType&& builder, PropType&& prop,
    MoreArgsType&&... moreArgs)
{
    using DecayType=typename std::decay<PropType>::type;
    ZCGUI_STATIC_ASSERT((std::is_base_of<props::Property, DecayType>::value),
        "Arguments must be widget properties or widgets.\n"
        "This may be a name collision.\n"
        "Is there something else with the same name in scope?");
    prop.assertUnique(psf);

    builder.applyProp(std::forward<PropType>(prop), PropType::tag);

    class PropsApplied: DecayType::TagType {};
    applyArgs(PropsApplied(), std::forward<BuilderType>(builder),
        std::forward<MoreArgsType>(moreArgs)...);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

} // namespace gui::internal

#endif
