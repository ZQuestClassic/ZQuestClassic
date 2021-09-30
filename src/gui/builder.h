#ifndef ZC_GUI_BUILDER_HPP
#define ZC_GUI_BUILDER_HPP

#include "button.h"
#include "checkbox.h"
#include "common.h"
#include "drop_down_list.h"
#include "grid.h"
#include "key.h"
#include "label.h"
#include "size.h"
#include "switcher.h"
#include "text_field.h"
#include "window.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

// These have to be included in order after the ones above.
#include "macros.h"
#include "props.h"

/*
 * A brief explanation of how this works:
 *
 * Each property name is a global variable of a unique type. That type has
 * an inner class template instantiated by its operator=. For instance,
 * the property checked is an instance of checkedProp, and checked=true
 * returns an instance of checkedProp::Value<bool>.
 *
 * The builder classes have a generic applyProp function that rejects any
 * argument as invalid. ZCGUI_ACCEPT_PROP adds a specialization for that
 * property that calls the corresponding function on the widget. Similarly,
 * the builders have an addChildren function that rejects all children.
 * ZCGUI_ACCEPT_ONE_CHILD and ZCGUI_ACCEPT_MULTIPLE_CHILDREN add overloads
 * that call the appropriate functions.
 *
 * A builder function calls the corresponding makeX function and creates
 * a builder class, then passes everything to the applyArgs functions.
 * If the first argument is a property, it calls the builder's applyProp
 * function and calls applyArgs with the remaining arguments. If it's a widget
 * pointer, it passes all the remaining arguments to addChildren.
 */

namespace GUI::Internal
{

struct DummyType {};
static constexpr DummyType dummy={};

// Regular widgets

inline std::shared_ptr<Button> makeButton()
{
	return std::make_shared<Button>();
}

inline std::shared_ptr<Checkbox> makeCheckbox()
{
	return std::make_shared<Checkbox>();
}

inline std::shared_ptr<DropDownList> makeDropDownList()
{
	return std::make_shared<DropDownList>();
}

inline std::shared_ptr<Label> makeLabel()
{
	return std::make_shared<Label>();
}

inline std::shared_ptr<TextField> makeTextField()
{
	return std::make_shared<TextField>();
}

// Containers

// This is counterintuitive: Multiple rows=rows, one row=columns.
inline std::shared_ptr<Grid> makeColumn()
{
	return Grid::rows(1);
}

inline std::shared_ptr<Grid> makeColumns(size_t size)
{
	return Grid::columns(size);
}

inline std::shared_ptr<Grid> makeRow()
{
	return Grid::columns(1);
}

inline std::shared_ptr<Grid> makeRows(size_t size)
{
	return Grid::rows(size);
}

inline std::shared_ptr<Switcher> makeSwitcher()
{
	return std::make_shared<Switcher>();
}

// Top-level widgets

inline std::shared_ptr<Window> makeWindow()
{
	return std::make_shared<Window>();
}

} // namespace GUI::Internal

namespace GUI::Builder
{

ZCGUI_BUILDER_START(Button)
	ZCGUI_ACCEPT_PROP(onClick, onClick, Dialog::message)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)

	ZCGUI_SUGGEST_PROP(title, text)
	ZCGUI_SUGGEST_PROP(onEnter, onClick)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Button, Button, makeButton)


ZCGUI_BUILDER_START(Checkbox)
	ZCGUI_ACCEPT_PROP(checked, setChecked, bool)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
	ZCGUI_ACCEPT_PROP(boxPlacement, setBoxPlacement, Checkbox::boxPlacement)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Checkbox, Checkbox, makeCheckbox)


ZCGUI_BUILDER_START(DropDownList)
	ZCGUI_ACCEPT_PROP(data, setListData, const ::GUI::ListData&)
	ZCGUI_ACCEPT_PROP(selectedValue, setSelectedValue, int)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)

	ZCGUI_SUGGEST_PROP(onClick, onSelectionChanged)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DropDownList, DropDownList, makeDropDownList)


ZCGUI_BUILDER_START(Label)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
	ZCGUI_ACCEPT_PROP(maxLines, setMaxLines, std::size_t)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Label, Label, makeLabel)


ZCGUI_BUILDER_START(Grid)
    ZCGUI_ACCEPT_PROP(columnSpacing, setColumnSpacing, ::GUI::Size)
    ZCGUI_ACCEPT_PROP(rowSpacing, setRowSpacing, ::GUI::Size)
    ZCGUI_ACCEPT_PROP(spacing, setSpacing, ::GUI::Size)

	ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Grid, Row, makeRow)
ZCGUI_BUILDER_FUNCTION_TEMPLATE(Grid, Rows, makeRows, std::size_t)
ZCGUI_BUILDER_FUNCTION(Grid, Column, makeColumn)
ZCGUI_BUILDER_FUNCTION_TEMPLATE(Grid, Columns, makeColumns, std::size_t)


ZCGUI_BUILDER_START(Switcher)
	ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Switcher, Switcher, makeSwitcher)


ZCGUI_BUILDER_START(TextField)
	ZCGUI_ACCEPT_PROP(maxLength, setMaxLength, std::size_t);
	ZCGUI_ACCEPT_PROP(onEnter, onEnter, Dialog::message)
	ZCGUI_ACCEPT_PROP(onValueChanged, onValueChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(text, setText, std::string_view)
	ZCGUI_ACCEPT_PROP(val, setVal, int)
	ZCGUI_ACCEPT_PROP(type, setType, TextField::type)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TextField, TextField, makeTextField)


ZCGUI_BUILDER_START(Window)
	ZCGUI_ACCEPT_PROP(title, setTitle, std::string)
	ZCGUI_ACCEPT_PROP(onClose, onClose, Dialog::message)
	ZCGUI_ACCEPT_PROP(onEnter, onEnter, Dialog::message)
	ZCGUI_ACCEPT_PROP(shortcuts, addShortcuts,
		std::initializer_list<KeyboardShortcut>)
	ZCGUI_ACCEPT_ONE_CHILD(setContent)

	ZCGUI_SUGGEST_PROP(text, title)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Window, Window, makeWindow)

} // namespace GUI::builder

namespace GUI::Props
{

// We want these in scope for setting sizes.
using ::GUI::operator ""_em;
using ::GUI::operator ""_px;
using ::GUI::operator ""_lpx;

} // namespace GUI::Props


#endif
