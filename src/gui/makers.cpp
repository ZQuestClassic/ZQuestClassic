#include "button.h"
#include "checkbox.h"
#include "dropDownList.h"
#include "grid.h"
#include "label.h"
#include "switcher.h"
#include "textField.h"
#include "window.h"
#include <memory>

namespace gui { namespace internal {

std::shared_ptr<Button> makeButton()
{
    return std::make_shared<Button>();
}

std::shared_ptr<Checkbox> makeCheckbox()
{
    return std::make_shared<Checkbox>();
}

std::shared_ptr<DropDownList> makeDropDownList()
{
    return std::make_shared<DropDownList>();
}

std::shared_ptr<Label> makeLabel()
{
    return std::make_shared<Label>();
}

std::shared_ptr<TextField> makeTextField()
{
    return std::make_shared<TextField>();
}

std::shared_ptr<Window> makeWindow()
{
    return std::make_shared<Window>();
}


// Containers


// This is counterintuitive: Multiple rows=Rows, one row=Columns.

std::shared_ptr<Grid> makeColumn()
{
    return Grid::rows(1);
}

std::shared_ptr<Grid> makeColumns(size_t size)
{
    return Grid::columns(size);
}

std::shared_ptr<Grid> makeRow()
{
    return Grid::columns(1);
}

std::shared_ptr<Grid> makeRows(size_t size)
{
    return Grid::rows(size);
}

std::shared_ptr<Switcher> makeSwitcher()
{
    return std::make_shared<Switcher>();
}

}} // namespace gui::internal
