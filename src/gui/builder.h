#ifndef ZC_GUI_BUILDER_HPP
#define ZC_GUI_BUILDER_HPP

#include "button.h"
#include "checkbox.h"
#include "dropDownList.h"
#include "grid.h"
#include "label.h"
#include "switcher.h"
#include "textField.h"
#include "window.h"
#include <memory>
#include <string>
#include <string_view>

// Depends on includes above. Hrm.
#include "macros.h"

namespace gui
{

namespace internal
{

struct DummyType {};
extern DummyType dummy; // Not defined anywhere

template<void(*)(DummyType&), bool b=false>
struct WidgetPtrType
{
    ZCGUI_STATIC_ASSERT(b, "Not a widget type");
    using WidgetType=int;
};

// TODO: Just inline these
std::shared_ptr<Button> makeButton();
std::shared_ptr<Checkbox> makeCheckbox();
std::shared_ptr<DropDownList> makeDropDownList();
std::shared_ptr<Label> makeLabel();
std::shared_ptr<TextField> makeTextField();
std::shared_ptr<Window> makeWindow();

std::shared_ptr<Grid> makeRow();
std::shared_ptr<Grid> makeRows(size_t size);
std::shared_ptr<Grid> makeColumn();
std::shared_ptr<Grid> makeColumns(size_t size);
std::shared_ptr<Switcher> makeSwitcher();

} // namespace internal

namespace builder
{

ZCGUI_BUILDER_START(Button)
    ZCGUI_ACCEPT_PROP(onClick, onClick, int)
    ZCGUI_ACCEPT_PROP(text, setText, std::string)

    ZCGUI_SUGGEST_PROP(title, text)
    ZCGUI_SUGGEST_PROP(onEnter, onClick)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Button, Button, internal::makeButton)


ZCGUI_BUILDER_START(Checkbox)
    ZCGUI_ACCEPT_PROP(checked, setChecked, bool)
    ZCGUI_ACCEPT_PROP(text, setText, std::string)
    ZCGUI_ACCEPT_PROP(boxPlacement, setBoxPlacement, Checkbox::BoxPlacement)

    ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Checkbox, Checkbox, internal::makeCheckbox)


ZCGUI_BUILDER_START(DropDownList)
    ZCGUI_ACCEPT_PROP(data, setListData, const gui::ListData&)
    ZCGUI_ACCEPT_PROP(selectedValue, setSelectedValue, int)
    ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, int)

    ZCGUI_SUGGEST_PROP(onClick, onSelectionChanged)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DropDownList, DropDownList, internal::makeDropDownList)


ZCGUI_BUILDER_START(Label)
    ZCGUI_ACCEPT_PROP(text, setText, std::string)

    ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Label, Label, internal::makeLabel)


ZCGUI_BUILDER_START(Grid)
    ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Grid, Row, internal::makeRow)
ZCGUI_BUILDER_FUNCTION_TEMPLATE(Grid, Rows, internal::makeRows, size_t)
ZCGUI_BUILDER_FUNCTION(Grid, Column, internal::makeColumn)
ZCGUI_BUILDER_FUNCTION_TEMPLATE(Grid, Columns, internal::makeColumns, size_t)


ZCGUI_BUILDER_START(Switcher)
    ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Switcher, Switcher, internal::makeSwitcher)


ZCGUI_BUILDER_START(TextField)
    ZCGUI_ACCEPT_PROP(maxLength, setMaxLength);
    ZCGUI_ACCEPT_PROP(onEnter, onEnter, int)
    ZCGUI_ACCEPT_PROP(onValueChanged, onValueChanged, int)
    ZCGUI_ACCEPT_PROP(text, setText, std::string_view)
    ZCGUI_ACCEPT_PROP(type, setType, TextField::Type)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TextField, TextField, internal::makeTextField)


ZCGUI_BUILDER_START(Window)
    ZCGUI_ACCEPT_PROP(title, setTitle, std::string)
    ZCGUI_ACCEPT_PROP(onClose, onClose, int)
    ZCGUI_ACCEPT_ONE_CHILD(setContent)

    ZCGUI_SUGGEST_PROP(text, title)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Window, Window, internal::makeWindow)

}} // namespace gui::builder

#define ZCGUI_USING_WIDGETS \
using gui::ZCGUI_WIDGET_NAME(Button);\
using gui::ZCGUI_WIDGET_NAME(Widget);\
using gui::ZCGUI_WIDGET_NAME(Window);\
using gui::ZCGUI_WIDGET_NAME(Label);\
using gui::ZCGUI_WIDGET_NAME(Column);\
using gui::ZCGUI_WIDGET_NAME(SerialContainer);

#endif
