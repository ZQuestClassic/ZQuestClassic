#ifndef ZC_GUI_BUILDER_HPP
#define ZC_GUI_BUILDER_HPP

#include "gui/use_size.h"
#include "gui/button.h"
#include "gui/checkbox.h"
#include "gui/checkbox_qr.h"
#include "gui/colorsel.h"
#include "gui/common.h"
#include "gui/ditherprev.h"
#include "gui/drop_down_list.h"
#include "gui/frame.h"
#include "gui/separator.h"
#include "gui/grid.h"
#include "gui/key.h"
#include "gui/label.h"
#include "gui/qrpanel.h"
#include "gui/ddpanel.h"
#include "gui/radio.h"
#include "gui/radioset.h"
#include "gui/size.h"
#include "gui/slider.h"
#include "gui/switcher.h"
#include "gui/tabpanel.h"
#include "gui/tabref.h"
#include "gui/list.h"
#include "gui/scrolling_pane.h"
#include "gui/text_field.h"
#include "gui/window.h"
#include "gui/widget.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

// These have to be included in order after the ones above.
#include "gui/macros.h"
#include "gui/props.h"

extern int32_t zq_screen_w, zq_screen_h;

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

inline std::shared_ptr<QRCheckbox> makeQRCheckbox()
{
	return std::make_shared<QRCheckbox>();
}

inline std::shared_ptr<DropDownList> makeDropDownList()
{
	return std::make_shared<DropDownList>();
}

inline std::shared_ptr<Label> makeLabel()
{
	return std::make_shared<Label>();
}

inline std::shared_ptr<List> makeList()
{
	return std::make_shared<List>();
}

inline std::shared_ptr<Separator> makeHSeparator()
{
	return std::make_shared<Separator>(false);
}

inline std::shared_ptr<Separator> makeVSeparator()
{
	return std::make_shared<Separator>(true);
}

inline std::shared_ptr<TextField> makeTextField()
{
	return std::make_shared<TextField>();
}

inline std::shared_ptr<ColorSel> makeColorSel()
{
	return std::make_shared<ColorSel>();
}

inline std::shared_ptr<Slider> makeSlider()
{
	return std::make_shared<Slider>();
}

inline std::shared_ptr<DitherPreview> makeDitherPreview()
{
	return std::make_shared<DitherPreview>();
}

// Containers

inline std::shared_ptr<Frame> makeFrame()
{
	return std::make_shared<Frame>();
}

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

inline std::shared_ptr<Grid> makeRowsColumns(size_t size, size_t size2)
{
	return Grid::rows_columns(size, size2);
}

inline std::shared_ptr<Grid> makeColumnsRows(size_t size, size_t size2)
{
	return Grid::columns_rows(size, size2);
}

inline std::shared_ptr<ScrollingPane> makeScrollingPane()
{
	return std::make_shared<ScrollingPane>();
}

inline std::shared_ptr<Switcher> makeSwitcher()
{
	return std::make_shared<Switcher>();
}

inline std::shared_ptr<Menu> makeMenu()
{
	return std::make_shared<Menu>();
}

inline std::shared_ptr<TabPanel> makeTabPanel()
{
	return std::make_shared<TabPanel>();
}

inline std::shared_ptr<QRPanel> makeQRPanel()
{
	return std::make_shared<QRPanel>();
}

inline std::shared_ptr<DDPanel> makeDDPanel()
{
	return std::make_shared<DDPanel>();
}

inline std::shared_ptr<TabRef> makeTabRef()
{
	return std::make_shared<TabRef>();
}

inline std::shared_ptr<Radio> makeRadio()
{
	return std::make_shared<Radio>();
}

inline std::shared_ptr<RadioSet> makeRadioSet()
{
	return std::make_shared<RadioSet>();
}

inline std::shared_ptr<DummyWidget> makeDummyWidget()
{
	return std::make_shared<DummyWidget>();
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
	ZCGUI_ACCEPT_PROP(onPressFunc, setOnPress, GUI::function<void()>)
	ZCGUI_ACCEPT_PROP(type, setType, Button::type)
	ZCGUI_ACCEPT_PROP(kb_ptr, setBoundKB, int*)
	ZCGUI_ACCEPT_PROP(hotkey_ptr, setBoundHotkey, Hotkey*)
	ZCGUI_ACCEPT_PROP(hotkey_indx, setHotkeyIndx, size_t)
	ZCGUI_ACCEPT_PROP(stick_index_ptr, setBoundStickIndex, int*)
	ZCGUI_ACCEPT_PROP(bind_name, setBindName, std::string)
	ZCGUI_ACCEPT_PROP(icon, setIcon, int)

	ZCGUI_SUGGEST_PROP(title, text)
	ZCGUI_SUGGEST_PROP(onEnter, onClick)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Button, Button, makeButton)


ZCGUI_BUILDER_START(Checkbox)
	ZCGUI_ACCEPT_PROP(checked, setChecked, bool)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
	ZCGUI_ACCEPT_PROP(boxPlacement, setBoxPlacement, Checkbox::boxPlacement)
	ZCGUI_ACCEPT_PROP(onToggle, onToggle, Dialog::message)
	ZCGUI_ACCEPT_PROP(onToggleFunc, setOnToggleFunc, GUI::function<void(bool)>)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Checkbox, Checkbox, makeCheckbox)

ZCGUI_BUILDER_START(Radio)
	ZCGUI_ACCEPT_PROP(checked, setChecked, bool)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
	ZCGUI_ACCEPT_PROP(set, setProcSet, int32_t)
	ZCGUI_ACCEPT_PROP(indx, setIndex, size_t)
	ZCGUI_ACCEPT_PROP(onToggle, onToggle, Dialog::message)
	ZCGUI_ACCEPT_PROP(onPressFunc, setOnPressFunc, GUI::function<void(size_t)>)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Radio, Radio, makeRadio)


ZCGUI_BUILDER_START(RadioSet)
	ZCGUI_ACCEPT_PROP(checked, setChecked, int32_t)
	ZCGUI_ACCEPT_PROP(set, setProcSet, int32_t)
	ZCGUI_ACCEPT_PROP(onToggle, onToggle, Dialog::message)
	
	ZCGUI_ACCEPT_PROP(data, loadList, const ::GUI::ListData&)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(RadioSet, RadioSet, makeRadioSet)


ZCGUI_BUILDER_START(DropDownList)
	ZCGUI_ACCEPT_PROP(data, setListData, const ::GUI::ListData&)
	ZCGUI_ACCEPT_PROP(selectedValue, setSelectedValue, int32_t)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(onSelectFunc, setOnSelectFunc, GUI::function<void(int32_t)>)

	ZCGUI_SUGGEST_PROP(onClick, onSelectionChanged)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DropDownList, DropDownList, makeDropDownList)


ZCGUI_BUILDER_START(Frame)
	ZCGUI_ACCEPT_PROP(title, setTitle, std::string)
	ZCGUI_ACCEPT_PROP(info, setInfo, std::string)
	ZCGUI_ACCEPT_PROP(style, setStyle, GUI::Frame::style)
	ZCGUI_ACCEPT_PROP(onInfo, onInfo, Dialog::message)
	ZCGUI_ACCEPT_ONE_CHILD(setContent)

	ZCGUI_SUGGEST_PROP(text, title)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Frame, Frame, makeFrame)


ZCGUI_BUILDER_START(Label)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)
	ZCGUI_ACCEPT_PROP(maxLines, setMaxLines, std::size_t)
	ZCGUI_ACCEPT_PROP(textAlign, setAlign, int32_t)
	ZCGUI_ACCEPT_PROP(textColor, setTextColor, int32_t)
	ZCGUI_ACCEPT_PROP(onPressFunc, setOnPress, GUI::function<void()>)
	ZCGUI_ACCEPT_PROP(noHLine, setNoHLine, bool)

	ZCGUI_SUGGEST_PROP(title, text)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Label, Label, makeLabel)


ZCGUI_BUILDER_START(List)
	ZCGUI_ACCEPT_PROP(data, setListData, GUI::ListData)
	ZCGUI_ACCEPT_PROP(selectedValue, setSelectedValue, int32_t)
	ZCGUI_ACCEPT_PROP(selectedIndex, setSelectedIndex, int32_t)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(onRClick, onRClick, Dialog::message)
	ZCGUI_ACCEPT_PROP(onDClick, onDClick, Dialog::message)
	ZCGUI_ACCEPT_PROP(onRClickFunc, setOnRClickFunc, GUI::function<void(int32_t,int32_t,int32_t)>)
	ZCGUI_ACCEPT_PROP(onDClickFunc, setOnDClickFunc, GUI::function<void(int32_t,int32_t,int32_t)>)
	ZCGUI_ACCEPT_PROP(onSelectFunc, setOnSelectFunc, GUI::function<void(int32_t)>)
	ZCGUI_ACCEPT_PROP(isABC, setIsABC, bool)
	ZCGUI_ACCEPT_PROP(scrollPtr, setScrollPtr, size_t*)

	ZCGUI_SUGGEST_PROP(onClick, onSelectionChanged)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(List, List, makeList)


ZCGUI_BUILDER_START(Separator)
	ZCGUI_ACCEPT_PROP(color_1, setColor1, byte)
	ZCGUI_ACCEPT_PROP(color_2, setColor2, byte)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Separator, HSeparator, makeHSeparator)
ZCGUI_BUILDER_FUNCTION(Separator, VSeparator, makeVSeparator)

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
ZCGUI_BUILDER_FUNCTION_TEMPLATE2(Grid, Rows_Columns, makeRowsColumns, std::size_t, std::size_t)
ZCGUI_BUILDER_FUNCTION_TEMPLATE2(Grid, Columns_Rows, makeColumnsRows, std::size_t, std::size_t)


ZCGUI_BUILDER_START(ScrollingPane)
	ZCGUI_ACCEPT_PROP(ptr_x, setPtrX, int32_t*)
	ZCGUI_ACCEPT_PROP(ptr_y, setPtrY, int32_t*)
	ZCGUI_ACCEPT_PROP(targWidth, setTargWidth, ::GUI::Size)
	ZCGUI_ACCEPT_PROP(targHeight, setTargHeight, ::GUI::Size)
	ZCGUI_ACCEPT_PROP(mode, setMode, ScrollingPane::Mode)
	ZCGUI_ACCEPT_ONE_CHILD(setContent)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(ScrollingPane, ScrollingPane, makeScrollingPane)


ZCGUI_BUILDER_START(Switcher)
	ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Switcher, Switcher, makeSwitcher)

ZCGUI_BUILDER_START(Menu)
	ZCGUI_ACCEPT_PROP(menu, setMenu, GuiMenu*)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Menu, Menu, makeMenu)

ZCGUI_BUILDER_START(TabPanel)
	ZCGUI_ACCEPT_PROP(indx, switchTo, size_t)
	ZCGUI_ACCEPT_PROP(ptr, setPtr, size_t*)
	ZCGUI_ACCEPT_PROP(onSwitch, setOnSwitch, GUI::function<void(size_t,size_t)>)
	ZCGUI_ACCEPT_MULTIPLE_CHILDREN(add)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TabPanel, TabPanel, makeTabPanel)

ZCGUI_BUILDER_START(QRPanel)
	ZCGUI_ACCEPT_PROP(onToggle, onToggle, Dialog::message)
	ZCGUI_ACCEPT_PROP(onCloseInfo, onCloseInfo, Dialog::message)
	ZCGUI_ACCEPT_PROP(qr_ptr, loadQRs, byte*)
	ZCGUI_ACCEPT_PROP(ptr, setPtr, size_t*)
	ZCGUI_ACCEPT_PROP(count, setCount, size_t)
	ZCGUI_ACCEPT_PROP(showtags, setShowTags, bool)
	ZCGUI_ACCEPT_PROP(indexed, setIndexed, bool)
	ZCGUI_ACCEPT_PROP(scrollWidth, setScrollWidth, ::GUI::Size)
	ZCGUI_ACCEPT_PROP(scrollHeight, setScrollHeight, ::GUI::Size)
	ZCGUI_ACCEPT_PROP(data, loadList, const ::GUI::ListData&)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(QRPanel, QRPanel, makeQRPanel)

ZCGUI_BUILDER_START(DDPanel)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(values, linkVals, int32_t*)
	ZCGUI_ACCEPT_PROP(count, setCount, size_t)
	ZCGUI_ACCEPT_PROP(ddlist, loadDDList, const ::GUI::ListData&)
	ZCGUI_ACCEPT_PROP(data, loadList, const ::GUI::ListData&)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DDPanel, DDPanel, makeDDPanel)

ZCGUI_BUILDER_START(TabRef)
	ZCGUI_ACCEPT_PROP(name, setName, std::string)
	ZCGUI_ACCEPT_ONE_CHILD(setContent)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TabRef, TabRef, makeTabRef)

ZCGUI_BUILDER_START(TextField)
	ZCGUI_ACCEPT_PROP(maxLength, setMaxLength, std::size_t);
	ZCGUI_ACCEPT_PROP(onEnter, onEnter, Dialog::message)
	ZCGUI_ACCEPT_PROP(onValueChanged, onValueChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(text, setText, std::string_view)
	ZCGUI_ACCEPT_PROP(val, setVal, int32_t)
	ZCGUI_ACCEPT_PROP(type, setType, TextField::type)
	ZCGUI_ACCEPT_PROP(bounds, setBounds, TextField::Bounds)
	ZCGUI_ACCEPT_PROP(onValChangedFunc, setOnValChanged, GUI::function<void(TextField::type,std::string_view,int32_t)>)
	ZCGUI_ACCEPT_PROP(onCursorChangedFunc, setOnCursorChanged, GUI::function<void(TextField::type,int32_t,int32_t)>)
	ZCGUI_ACCEPT_PROP(places, setFixedPlaces, size_t)
	ZCGUI_ACCEPT_PROP(swap_type, setSwapType, int32_t)
	ZCGUI_ACCEPT_PROP(msgStrPaste, setMsgStrPaste, bool)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TextField, TextField, makeTextField)

ZCGUI_BUILDER_START(ColorSel)
	ZCGUI_ACCEPT_PROP(onValueChanged, onValueChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(val, setVal, byte)
	ZCGUI_ACCEPT_PROP(onValChangedFunc, setOnValChanged, GUI::function<void(byte)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(ColorSel, ColorSel, makeColorSel)

ZCGUI_BUILDER_START(Slider)
	ZCGUI_ACCEPT_PROP(onValueChanged, onValueChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(value, setValue, int32_t)
	ZCGUI_ACCEPT_PROP(min_value, setMinValue, int32_t)
	ZCGUI_ACCEPT_PROP(max_value, setMaxValue, int32_t)
	ZCGUI_ACCEPT_PROP(step, setStep, int32_t)
	ZCGUI_ACCEPT_PROP(handle_width, setHandleWidth, ::GUI::Size)
	ZCGUI_ACCEPT_PROP(bar_width, setBarWidth, ::GUI::Size)
	ZCGUI_ACCEPT_PROP(onValChangedFunc, setOnValChanged, GUI::function<void(int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Slider, Slider, makeSlider)

ZCGUI_BUILDER_START(DitherPreview)
	ZCGUI_ACCEPT_PROP(dither_type, setDitherType, byte)
	ZCGUI_ACCEPT_PROP(dither_arg, setDitherArg, byte)
	ZCGUI_ACCEPT_PROP(dither_color, setDitherColor, byte)
	ZCGUI_ACCEPT_PROP(preview_scale, setPreviewScale, byte)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DitherPreview, DitherPreview, makeDitherPreview)

ZCGUI_BUILDER_START(Window)
	ZCGUI_ACCEPT_PROP(title, setTitle, std::string)
	ZCGUI_ACCEPT_PROP(info, setHelp, std::string)
	ZCGUI_ACCEPT_PROP(menu, setMenu, std::shared_ptr<Menu>)
	ZCGUI_ACCEPT_PROP(onClose, onClose, Dialog::message)
	ZCGUI_ACCEPT_PROP(onEnter, onEnter, Dialog::message)
	ZCGUI_ACCEPT_PROP(use_vsync, setVSync, bool)
	ZCGUI_ACCEPT_PROP(shortcuts, addShortcuts,
		std::initializer_list<KeyboardShortcut>)
	ZCGUI_ACCEPT_PROP(onTick, setOnTick, GUI::function<int32_t()>)
	ZCGUI_ACCEPT_PROP(onLoad, setOnLoad, GUI::function<void()>)
	ZCGUI_ACCEPT_ONE_CHILD(setContent)

	ZCGUI_SUGGEST_PROP(text, title)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Window, Window, makeWindow)

ZCGUI_BUILDER_START(DummyWidget)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DummyWidget, DummyWidget, makeDummyWidget)

} // namespace GUI::builder

namespace GUI::Props
{

// We want these in scope for setting sizes.
using ::GUI::operator ""_em;
using ::GUI::operator ""_px;

} // namespace GUI::Props

//{ Builder Shortcuts
#define _d DummyWidget()

//

#define INITD_ROW(ind, d_mem, lab_mem) \
Row(vPadding = 0_px, \
	TextField(maxLength = 64, \
		text = std::string(lab_mem[ind]), \
		type = GUI::TextField::type::TEXT, \
		width = 8_em, \
		rightPadding = 0_px, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t) \
		{ \
			for(size_t q = 0; q < 65; ++q) \
			{ \
				if(q < str.size()) \
					lab_mem[ind][q] = str.at(q); \
				else \
					lab_mem[ind][q] = 0; \
			} \
		} \
	), \
	TextField( \
		type = GUI::TextField::type::SWAP_ZSINT, \
		val = d_mem[ind], \
		width = 6.5_em+16_px, \
		leftPadding = 0_px, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
		{ \
			d_mem[ind] = val; \
		} \
	) \
)

#define INITD_ROW2(ind, d_mem) \
Row(vPadding = 0_px, \
	Label( text = "InitD["+std::to_string(ind)+"]:", \
		textAlign = 2, \
		rightPadding = 0_px \
	), \
	TextField( \
		type = GUI::TextField::type::SWAP_ZSINT, \
		val = d_mem[ind], \
		width = 6.5_em+16_px, \
		leftPadding = 0_px, \
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
		{ \
			d_mem[ind] = val; \
		} \
	) \
)

#define SCRIPT_LIST(name, list, mem) \
Label(minwidth = 6.5_em, text = name, textAlign = 2), \
DropDownList( \
	fitParent = true, \
	data = list, \
	selectedValue = mem, \
	onSelectFunc = [&](int32_t val) \
	{ \
		mem = val; \
	} \
)

#define SCRIPT_LIST_PROC(name, list, mem, proc) \
Label(minwidth = 6.5_em, text = name, textAlign = 2), \
DropDownList( \
	fitParent = true, \
	data = list, \
	selectedValue = mem, \
	onSelectFunc = [&](int32_t val) \
	{ \
		mem = val; \
		proc(); \
	} \
)

//

#define DINFOBTN() \
Button(forceFitH = true, text = "?", \
	disabled = true)
#define INFOBTN(inf) \
Button(forceFitH = true, text = "?", \
	onClick = message::REFR_INFO, \
	onPressFunc = [=, this]() \
	{ \
		InfoDialog("Info",inf).show(); \
	})
#define INFOBTN_VS(inf, vs) \
Button(forceFitH = true, text = "?", \
	vPadding = vs, \
	onClick = message::REFR_INFO, \
	onPressFunc = [=, this]() \
	{ \
		InfoDialog("Info",inf).show(); \
	})
#define INFOBTN_T(title, inf) \
Button(forceFitH = true, text = "?", \
	onClick = message::REFR_INFO, \
	onPressFunc = [=, this]() \
	{ \
		InfoDialog(title,inf).show(); \
	})
#define INFOBTN_F(inf,...) \
Button(forceFitH = true, text = "?", \
	onClick = message::REFR_INFO, \
	onPressFunc = [=, this]() \
	{ \
		InfoDialog("Info",fmt::format(inf,__VA_ARGS__)).show(); \
	})
#define INFOBTN_EX(inf, ...) \
Button(text = "?", \
	onClick = message::REFR_INFO, \
	__VA_ARGS__, \
	onPressFunc = [=, this]() \
	{ \
		InfoDialog("Info",inf).show(); \
	})
#define INFOBTN_T_EX(title, inf, ...) \
Button(text = "?", \
	onClick = message::REFR_INFO, \
	__VA_ARGS__, \
	onPressFunc = [=, this]() \
	{ \
		InfoDialog(title,inf).show(); \
	})
#define INFOBTN_REF(getter) \
Button(forceFitH = true, text = "?", \
	onClick = message::REFR_INFO, \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",getter).show(); \
	})

//

#define _INTBTN_PANEL_HELPER(var) \
Columns<4>( \
	Checkbox( \
		text = "A", hAlign = 0.0, \
		checked = var&0x1, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x1,state); \
		}), \
	Checkbox( \
		text = "B", hAlign = 0.0, \
		checked = var&0x2, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x2,state); \
		}), \
	Checkbox( \
		text = "L", hAlign = 0.0, \
		checked = var&0x4, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x4,state); \
		}), \
	Checkbox( \
		text = "R", hAlign = 0.0, \
		checked = var&0x8, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x8,state); \
		}), \
	Checkbox( \
		text = "Ex1", hAlign = 0.0, \
		checked = var&0x10, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x10,state); \
		}), \
	Checkbox( \
		text = "Ex2", hAlign = 0.0, \
		checked = var&0x20, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x20,state); \
		}), \
	Checkbox( \
		text = "Ex3", hAlign = 0.0, \
		checked = var&0x40, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x40,state); \
		}), \
	Checkbox( \
		text = "Ex4", hAlign = 0.0, \
		checked = var&0x80, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(var,0x80,state); \
		}) \
)
#define _INTBTN_PANEL_HELPER_CAPTURE(var, ...) \
Columns<4>( \
	Checkbox( \
		text = "A", hAlign = 0.0, \
		checked = var&0x1, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x1,state); \
		}), \
	Checkbox( \
		text = "B", hAlign = 0.0, \
		checked = var&0x2, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x2,state); \
		}), \
	Checkbox( \
		text = "L", hAlign = 0.0, \
		checked = var&0x4, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x4,state); \
		}), \
	Checkbox( \
		text = "R", hAlign = 0.0, \
		checked = var&0x8, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x8,state); \
		}), \
	Checkbox( \
		text = "Ex1", hAlign = 0.0, \
		checked = var&0x10, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x10,state); \
		}), \
	Checkbox( \
		text = "Ex2", hAlign = 0.0, \
		checked = var&0x20, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x20,state); \
		}), \
	Checkbox( \
		text = "Ex3", hAlign = 0.0, \
		checked = var&0x40, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x40,state); \
		}), \
	Checkbox( \
		text = "Ex4", hAlign = 0.0, \
		checked = var&0x80, \
		onToggleFunc = [&, __VA_ARGS__](bool state) \
		{ \
			SETFLAG(var,0x80,state); \
		}) \
)

#define INTBTN_PANEL_EX(var,ltxt,inftxt,...) \
Column(padding = 0_px, __VA_ARGS__, \
	Row(padding = 0_px, \
		Label(text = ltxt), \
		INFOBTN(inftxt) \
	), \
	_INTBTN_PANEL_HELPER(var) \
)
#define INTBTN_PANEL(var,ltxt,inftxt) \
Column(padding = 0_px, \
	Row(padding = 0_px, \
		Label(text = ltxt), \
		INFOBTN(inftxt) \
	), \
	_INTBTN_PANEL_HELPER(var) \
)

#define INTBTN_FRAME_EX(var,ltxt,inftxt,...) \
Frame(title = ltxt, __VA_ARGS__, \
	info = inftxt, \
	_INTBTN_PANEL_HELPER(var) \
)
#define INTBTN_FRAME(var,ltxt,inftxt) \
Frame(title = ltxt, \
	info = inftxt, \
	_INTBTN_PANEL_HELPER(var) \
)

//

#define _INTBTN_PANEL_HELPER2(var,ltxt) \
	Label(text = ltxt), \
	_INTBTN_PANEL_HELPER(var) \
)
#define INTBTN_PANEL2_EX(var,ltxt,...) \
Column(padding = 0_px, __VA_ARGS__, _INTBTN_PANEL_HELPER2(var,ltxt)
#define INTBTN_PANEL2(var,ltxt) \
Column(padding = 0_px, _INTBTN_PANEL_HELPER2(var,ltxt)

//

#define _EX_RBOX hAlign = 1.0,boxPlacement = GUI::Checkbox::boxPlacement::RIGHT

//}

#endif
