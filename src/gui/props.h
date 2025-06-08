#ifndef ZCGUI_PROPS_H_
#define ZCGUI_PROPS_H_

namespace GUI::Props
{

struct Property {};

// gcc -Weffc++ will complain "'operator=' should return a reference to '*this'"
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

ZCGUI_DECLARE_PROPERTY(bottomMargin)
ZCGUI_DECLARE_PROPERTY(bottomPadding)
ZCGUI_DECLARE_PROPERTY(boxPlacement)
ZCGUI_DECLARE_PROPERTY(checked)
ZCGUI_DECLARE_PROPERTY(columnSpacing)
ZCGUI_DECLARE_PROPERTY(onSwitch)
ZCGUI_DECLARE_PROPERTY(count)
ZCGUI_DECLARE_PROPERTY(bitmap)
ZCGUI_DECLARE_PROPERTY(cdata)
ZCGUI_DECLARE_PROPERTY(palette)
ZCGUI_DECLARE_PROPERTY(data)
ZCGUI_DECLARE_PROPERTY(focused)
ZCGUI_DECLARE_PROPERTY(hAlign)
ZCGUI_DECLARE_PROPERTY(height)
ZCGUI_DECLARE_PROPERTY(maxheight)
ZCGUI_DECLARE_PROPERTY(minheight)
ZCGUI_DECLARE_PROPERTY(hMargins)
ZCGUI_DECLARE_PROPERTY(hPadding)
ZCGUI_DECLARE_PROPERTY(initializer)
ZCGUI_DECLARE_PROPERTY(indexed)
ZCGUI_DECLARE_PROPERTY(leftMargin)
ZCGUI_DECLARE_PROPERTY(leftPadding)
ZCGUI_DECLARE_PROPERTY(margins)
ZCGUI_DECLARE_PROPERTY(padding)
ZCGUI_DECLARE_PROPERTY(maxLength)
ZCGUI_DECLARE_PROPERTY(maxLines)
ZCGUI_DECLARE_PROPERTY(onClick)
ZCGUI_DECLARE_PROPERTY(onInfo)
ZCGUI_DECLARE_PROPERTY(onClose)
ZCGUI_DECLARE_PROPERTY(onSelectionChanged)
ZCGUI_DECLARE_PROPERTY(onValueChanged)
ZCGUI_DECLARE_PROPERTY(onEnter)
ZCGUI_DECLARE_PROPERTY(onToggle)
ZCGUI_DECLARE_PROPERTY(rightMargin)
ZCGUI_DECLARE_PROPERTY(rightPadding)
ZCGUI_DECLARE_PROPERTY(rowSpacing)
ZCGUI_DECLARE_PROPERTY(selectedValue)
ZCGUI_DECLARE_PROPERTY_AS(shortcuts, std::initializer_list<KeyboardShortcut>)
ZCGUI_DECLARE_PROPERTY(spacing)
ZCGUI_DECLARE_PROPERTY(style)
ZCGUI_DECLARE_PROPERTY(text)
ZCGUI_DECLARE_PROPERTY(title)
ZCGUI_DECLARE_PROPERTY(topMargin)
ZCGUI_DECLARE_PROPERTY(topPadding)
ZCGUI_DECLARE_PROPERTY(type)
ZCGUI_DECLARE_PROPERTY(userData)
ZCGUI_DECLARE_PROPERTY(vAlign)
ZCGUI_DECLARE_PROPERTY(visible)
ZCGUI_DECLARE_PROPERTY(vMargins)
ZCGUI_DECLARE_PROPERTY(vPadding)
ZCGUI_DECLARE_PROPERTY(width)
ZCGUI_DECLARE_PROPERTY(maxwidth)
ZCGUI_DECLARE_PROPERTY(minwidth)
ZCGUI_DECLARE_PROPERTY(val)
ZCGUI_DECLARE_PROPERTY(name)
ZCGUI_DECLARE_PROPERTY(set)
ZCGUI_DECLARE_PROPERTY(indx)
ZCGUI_DECLARE_PROPERTY(framed)
ZCGUI_DECLARE_PROPERTY(nopad)
ZCGUI_DECLARE_PROPERTY(fitParent)
ZCGUI_DECLARE_PROPERTY(textAlign)
ZCGUI_DECLARE_PROPERTY(disabled)
ZCGUI_DECLARE_PROPERTY(onPressFunc)
ZCGUI_DECLARE_PROPERTY(low)
ZCGUI_DECLARE_PROPERTY(high)
ZCGUI_DECLARE_PROPERTY(useFont)
ZCGUI_DECLARE_PROPERTY(frameText)
ZCGUI_DECLARE_PROPERTY(onValChangedFunc)
ZCGUI_DECLARE_PROPERTY(places)
ZCGUI_DECLARE_PROPERTY(swap_type)
ZCGUI_DECLARE_PROPERTY(onToggleFunc)
ZCGUI_DECLARE_PROPERTY(onSelectFunc)
ZCGUI_DECLARE_PROPERTY(onRClickFunc)
ZCGUI_DECLARE_PROPERTY(onDClickFunc)
ZCGUI_DECLARE_PROPERTY(onRClick)
ZCGUI_DECLARE_PROPERTY(onDClick)
ZCGUI_DECLARE_PROPERTY(selectedIndex)
ZCGUI_DECLARE_PROPERTY(isABC)
ZCGUI_DECLARE_PROPERTY(cset)
ZCGUI_DECLARE_PROPERTY(tile)
ZCGUI_DECLARE_PROPERTY(use_vsync)
ZCGUI_DECLARE_PROPERTY(speed)
ZCGUI_DECLARE_PROPERTY(frames)
ZCGUI_DECLARE_PROPERTY(delay)
ZCGUI_DECLARE_PROPERTY(skipx)
ZCGUI_DECLARE_PROPERTY(skipy)
ZCGUI_DECLARE_PROPERTY(do_sized)
ZCGUI_DECLARE_PROPERTY(flip)
ZCGUI_DECLARE_PROPERTY(forceFitW)
ZCGUI_DECLARE_PROPERTY(forceFitH)
ZCGUI_DECLARE_PROPERTY(rowSpan)
ZCGUI_DECLARE_PROPERTY(colSpan)
ZCGUI_DECLARE_PROPERTY(info)
ZCGUI_DECLARE_PROPERTY(combo)
ZCGUI_DECLARE_PROPERTY(showvals)
ZCGUI_DECLARE_PROPERTY(color)
ZCGUI_DECLARE_PROPERTY(cset2)
ZCGUI_DECLARE_PROPERTY(flashcs)
ZCGUI_DECLARE_PROPERTY(ptr)
ZCGUI_DECLARE_PROPERTY(values)
ZCGUI_DECLARE_PROPERTY(ddlist)
ZCGUI_DECLARE_PROPERTY(onUpdate)
ZCGUI_DECLARE_PROPERTY(onTick)
ZCGUI_DECLARE_PROPERTY(onLoad)
ZCGUI_DECLARE_PROPERTY(read_only)
ZCGUI_DECLARE_PROPERTY(noHLine)
ZCGUI_DECLARE_PROPERTY(showFlip)
ZCGUI_DECLARE_PROPERTY(c1)
ZCGUI_DECLARE_PROPERTY(c2)
ZCGUI_DECLARE_PROPERTY(system)
ZCGUI_DECLARE_PROPERTY(tilewid)
ZCGUI_DECLARE_PROPERTY(tilehei)
ZCGUI_DECLARE_PROPERTY(mini)
ZCGUI_DECLARE_PROPERTY(minicorner)
ZCGUI_DECLARE_PROPERTY(showT0)
ZCGUI_DECLARE_PROPERTY(minionly)
ZCGUI_DECLARE_PROPERTY(deftile)
ZCGUI_DECLARE_PROPERTY(defcs)
ZCGUI_DECLARE_PROPERTY(scrollWidth)
ZCGUI_DECLARE_PROPERTY(scrollHeight)
ZCGUI_DECLARE_PROPERTY(targHeight)
ZCGUI_DECLARE_PROPERTY(kb_ptr)
ZCGUI_DECLARE_PROPERTY(hotkey_ptr)
ZCGUI_DECLARE_PROPERTY(hotkey_indx)
ZCGUI_DECLARE_PROPERTY(icon)
ZCGUI_DECLARE_PROPERTY(showtags)
ZCGUI_DECLARE_PROPERTY(onCloseInfo)
ZCGUI_DECLARE_PROPERTY(curMap)
ZCGUI_DECLARE_PROPERTY(smallDMap)
ZCGUI_DECLARE_PROPERTY(offset)
ZCGUI_DECLARE_PROPERTY(minOffset)
ZCGUI_DECLARE_PROPERTY(maxOffset)
ZCGUI_DECLARE_PROPERTY(mapGridPtr)
ZCGUI_DECLARE_PROPERTY(continueScreen)
ZCGUI_DECLARE_PROPERTY(compassScreen)
ZCGUI_DECLARE_PROPERTY(dither_type)
ZCGUI_DECLARE_PROPERTY(dither_arg)
ZCGUI_DECLARE_PROPERTY(dither_color)
ZCGUI_DECLARE_PROPERTY(preview_scale)
ZCGUI_DECLARE_PROPERTY(menu)
ZCGUI_DECLARE_PROPERTY(regionMapPtr)
ZCGUI_DECLARE_PROPERTY(localRegionsData)
ZCGUI_DECLARE_PROPERTY(heroSprite)
ZCGUI_DECLARE_PROPERTY(dir)


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

} // namespace GUI::Props

namespace GUI::Internal
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
    using DecayType = typename std::decay_t<PropType>;
    ZCGUI_STATIC_ASSERT((std::is_base_of_v<Props::Property, DecayType>),
        "Arguments must be widget properties or widgets.");
    prop.assertUnique(psf);

    builder.applyProp(std::forward<PropType>(prop), PropType::tag);
}

template<typename PropsSoFar, typename BuilderType, typename PropType, typename... MoreArgsType>
inline void applyArgs(PropsSoFar psf, BuilderType&& builder, PropType&& prop,
    MoreArgsType&&... moreArgs)
{
    using DecayType = typename std::decay_t<PropType>;
    ZCGUI_STATIC_ASSERT((std::is_base_of_v<Props::Property, DecayType>),
        "Arguments must be widget properties or widgets." ZCGUI_NEWLINE
        "This may be a name collision." ZCGUI_NEWLINE
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

} // namespace GUI::Internal

#endif
