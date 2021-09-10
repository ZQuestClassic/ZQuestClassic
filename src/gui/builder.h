#ifndef ZC_GUI_BUILDER_HPP
#define ZC_GUI_BUILDER_HPP

#include "button.h"
#include "checkbox.h"
#include "common.h"
#include "drop_down_list.h"
#include "grid.h"
#include "key.h"
#include "label.h"
#include "switcher.h"
#include "text_field.h"
#include "window.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

// Depends on includes above. Hrm.
#include "macros.h"

namespace GUI
{

namespace Internal
{

struct DummyType {};
extern DummyType dummy; // Not defined anywhere

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

} // namespace Internal

namespace Builder
{

ZCGUI_BUILDER_START(Button)
	ZCGUI_ACCEPT_PROP(onClick, onClick, int)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)

	ZCGUI_SUGGEST_PROP(title, text)
	ZCGUI_SUGGEST_PROP(onEnter, onClick)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Button, Button, Internal::makeButton)


ZCGUI_BUILDER_START(Checkbox)
	ZCGUI_ACCEPT_PROP(checked, setChecked, bool)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
	ZCGUI_ACCEPT_PROP(boxPlacement, setBoxPlacement, Checkbox::BoxPlacement)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Checkbox, Checkbox, Internal::makeCheckbox)


ZCGUI_BUILDER_START(DropDownList)
	ZCGUI_ACCEPT_PROP(data, setListData, const GUI::ListData&)
	ZCGUI_ACCEPT_PROP(selectedValue, setSelectedValue, int)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, int)

	ZCGUI_SUGGEST_PROP(onClick, onSelectionChanged)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DropDownList, DropDownList, Internal::makeDropDownList)


ZCGUI_BUILDER_START(Label)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
		ZCGUI_ACCEPT_PROP(maxLines, setMaxLines, size_t)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Label, Label, Internal::makeLabel)


ZCGUI_BUILDER_START(Grid)
	ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Grid, Row, Internal::makeRow)
ZCGUI_BUILDER_FUNCTION_TEMPLATE(Grid, Rows, Internal::makeRows, size_t)
ZCGUI_BUILDER_FUNCTION(Grid, Column, Internal::makeColumn)
ZCGUI_BUILDER_FUNCTION_TEMPLATE(Grid, Columns, Internal::makeColumns, size_t)


ZCGUI_BUILDER_START(Switcher)
	ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Switcher, Switcher, Internal::makeSwitcher)


ZCGUI_BUILDER_START(TextField)
	ZCGUI_ACCEPT_PROP(maxLength, setMaxLength, size_t);
	ZCGUI_ACCEPT_PROP(onEnter, onEnter, int)
	ZCGUI_ACCEPT_PROP(onValueChanged, onValueChanged, int)
	ZCGUI_ACCEPT_PROP(text, setText, std::string_view)
	ZCGUI_ACCEPT_PROP(type, setType, TextField::Type)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TextField, TextField, Internal::makeTextField)


ZCGUI_BUILDER_START(Window)
	ZCGUI_ACCEPT_PROP(title, setTitle, std::string)
	ZCGUI_ACCEPT_PROP(onClose, onClose, int)
	ZCGUI_ACCEPT_PROP(shortcuts, addShortcuts,
		std::initializer_list<KeyboardShortcut>)
	ZCGUI_ACCEPT_ONE_CHILD(setContent)

	ZCGUI_SUGGEST_PROP(text, title)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Window, Window, Internal::makeWindow)

} // namespace builder

namespace Props
{

// Handy to have these in scope for setting sizes.
using ::GUI::operator ""_em;
using ::GUI::operator ""_px;
using ::GUI::operator ""_lpx;

}}


#endif
