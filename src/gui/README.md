## Quick overview

A dialog is created as a subclass of `Dialog`. Its `view` function creates the window, which is made up of subclasses of `Widget`. The dialog's `show` function creates a `DialogRunner`, which builds and displays a list of Allegro `DIALOG`s from the window. The `DIALOG`s' `proc`s use the `GUI_EVENT` macro to notify the `DialogRunner` when an event of interest occurs. The `DialogRunner` finds the corresponding `Widget` and notifies it of the event, prompting it to send a message to the dialog if one has been set.

`builder.h` define a simple GUI builder system that is illustrated in `dialog/README.md`.

## Creating a new Widget class

*Please note that this is still subject to change*

There are several functions `Widget` subclasses should override. All subclasses *must* override the first two.

```
void realize(DialogRunner& runner)
```
This function creates the `DIALOG` using `DialogRunner::push()`. That function returns an instance of `DialogRef` that can be used as a pointer to the new `DIALOG`, which should be stored for future modifications.

The first argument to `push()` should be `shared_from_this()` so that the DialogRunner knows which `Widget` owns the `DIALOG`. The `DIALOG`'s `flags` should be initialized with `getFlags()` plus whatever the widget's functionality requires. If for some reason the number of `DIALOG`s needed might vary over time, the maximum possible number should be added, with any unneeded ones being filled with placeholders.

Containers should propagate the call to their children. Top-level widgets should call `realizeKeys()` after adding their own and their children's `DIALOG`s.


For example, the implementation in `Checkbox` looks like this:
```
void Checkbox::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		jwin_checkfont_proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		getAccelKey(text),
		getFlags()|(checked ? D_SELECTED : 0),
		static_cast<int>(placement), 0,
		text.data(), widgFont, nullptr
	});
}
```

```
void applyVisibility(bool visible)
```
This makes the widget visible or invisible. Widgets with `DIALOG`s should set or unset the `D_HIDDEN` flag. Containers should propagate the call to their contents.

```
int onEvent(int event, MessageDispatcher& sendMessage)
```
This is called by the `DialogRunner` when the widget should send a message to the dialog. `sendMessage` should be called as a function with the message and argument as arguments. If there is no argument to send, use `MessageArg::none`.

Top-level widgets, if they don't recognize the event themselves, should call `TopLevelWidget::onEvent()`, which handles keyboard shortcuts.

This is the implementation in `Checkbox`:
```
int Checkbox::onEvent(int event, MessageDispatcher sendMessage)
{
	assert(event == ngeTOGGLE); // The only valid event for a Checkbox
	if(message >= 0) // -1 indicates no message has been set
		sendMessage(message, (alDialog->flags&D_SELECTED) != 0);
	return -1; // A negative return value means the DIALOG proc should keep running
}
```
Typically, the `event` argument should be from the `newGuiEvent` enum in jwin.h. Just add a new item if there isn't an appropriate one already. The event is triggered by the macro `GUI_EVENT`, which also immediately returns if the value returned by `onEvent` is not negative. `UI_EVENT` can simply be inserted into existing `proc` functions wherever the corresponding events occur. For instance, in `d_checkbox_proc`:
```
case MSG_KEY:

    /* close dialog? */
    if(d->flags & D_EXIT)
    {
        return D_CLOSE;
    }

    /* or just toggle */
    d->flags ^= D_SELECTED;
    GUI_EVENT(d, ngeTOGGLE);
    object_message(d, MSG_DRAW, 0);
    break;
```
If a new message argument type is needed, add it in dialog_message.h. It should be included in the inner `std::variant` and given a corresponding constructor and cast operator. Message arguments should be small enough to pass by value.

```
void calculateSize()
```
This can determine the preferred size of the widget, which should be set with `setPreferredWidth()` and `setPreferredHeight()`. However, many widgets determine their size when they're created or when their content is set, so they don't need this function. Containers should propagate the call to their children and use the result to set their own preferred sizes.

```
void arrange(int contX, int contY, int contW, int contH)
```
This sets the *actual* size of the widget and positions it in its parent. Non-container widgets should not override this unless they expect their size and position to change later on. Containers should call up to `Widget::arrange()`, then call `arrange()` for each of their children with appropriate areas of their own geometry.

## Adding a widget to the GUI builder system

In `builder.h`, add a maker function at the top of the file.
```
inline std::shared_ptr<Button> makeButton()
{
	return std::make_shared<Button>();
}
```
At the bottom, use the `ZCGUI_BUILDER` macros to define the builder and the properties it accepts. It's easiest to copy an existing one.
```
// Defines a class that builds a Button.
ZCGUI_BUILDER_START(Button)

	// The first argument is the property name. The second is the function to call
	// on the widget to set that property. After that comes one or more arguments
	// specifying the value types accepted. The actual types provided don't
	// have to match exactly; it just has to be possible to cast them.
	ZCGUI_ACCEPT_PROP(onClick, onClick, Dialog::message)
	ZCGUI_ACCEPT_PROP(text, setText, std::string)

	// ZCGUI_SUGGEST_PROP is used to output better error messages if an invalid
	// property is used. If you try to set a button's title, you'll get this error:
	// "Property 'title' is not valid for this widget. Did you mean 'text'?"
	ZCGUI_SUGGEST_PROP(title, text)
	ZCGUI_SUGGEST_PROP(onEnter, onClick)
ZCGUI_BUILDER_END()

// Defines the function template that will use the class defined above
// to create a button. The first argument is the widget type, the second
// is the name of the class declared above, and the third is the function
// that creates the widget.
ZCGUI_BUILDER_FUNCTION(Button, Button, makeButton)
```
If the new widget has a property not already named in props.h, add it there. It's a good idea to make sure everything still compiles afterward, as adding a new property could create a name conflict in existing dialogs.
