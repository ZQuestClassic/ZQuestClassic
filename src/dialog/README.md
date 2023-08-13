## Implementing a dialog

Every dialog should be a subclass of `gui::Dialog` (from `gui/dialog.h`) with itself as the template parameter. It needs an inner type called `message` that can safely be cast to an int32_t and back. Generally, this is an enum. It must not use negative values; these are used to represent no value internally. An extremely simple dialog that needs no messages can simply typedef `int32_t`.

A minimal dialog looks like this:
```
#include <gui/dialog.h>

class MyDialog: public GUI::Dialog<MyDialog>
{
public:
    enum class message
    {
        REFR_INFO, OK, CANCEL
    };

    std::shared_ptr<GUI::Widget> view() override;
    bool handleMessage(const GUI::DialogMessage<message>& msg);
};
```
Note that the `handleMessage` function is not virtual.

The `view` function returns the dialog's window. If the dialog needs to keep pointers to any widgets, that is also done here.

The GUI builder system allows you to design the window in a simple and natural way. To use it, include `gui/builder.h`. Inside `view`, you should use `using namespace GUI::Builder`, `using namespace GUI::Props`, and, if you want keyboard shortcuts, `using GUI::Key`.
```
std::shared_ptr<GUI::Widget> WhateverDialog::view()
{
    using namespace GUI::Builder;
    using namespace GUI::Props;
    using namespace GUI::Key;

    return Window(
        title="Some Dialog",
        onClose=message::CANCEL, // Sent when the close button is clicked or Escape is pressed

        shortcuts={
            F1=message::HELP,
        },

        Column(
            Row(
                Label(text="Enter a number:"),
                TextField(
                    type=GUI::TextField::type::INT_DECIMAL,
                    maxLength=5,
                    text=std::to_string(initialValue),
                    onValueChanged=message::SET_VALUE // Sent whenever the text changes
                )
            ),

            // If you need to keep a reference to a widget, you can store it
            // in a member variable at the same time it's created. The type of
            // this variable would be std::shared_ptr<GUI::Label>.
            warningLabel=Label(
                text="Value must not be negative",
                visible=value<0),

            Row(
                Button(
                    text="&Ok", // Ampersands automatically create keyboard shortcuts
                    onClick=message::OK
                ),
                Button(
                    text="&Cancel",
                    onClick=message::CANCEL
                )
            )
        )
    );
}
```
One potential pitfall in this system is name conflicts. If, for instance, the class has a member named `title`, that will take precedence over the window property, and the dialog will fail to compile. In these cases, you can rename the member to eliminate the conflict, specify `GUI::Props::title`, or move some or all of the window creation to a non-member function where the name is not in scope.

It's not necessary to define the entire layout in one big function call. Splitting the layout into multiple functions, or even multiple classes, is perfectly fine. Furthermore, it's not necessary to use this system at all. There are ordinary classes and functions underneath that can be used directly.
```
std::shared_ptr<GUI::Widget> WhateverDialog::view()
{
    auto win=std::make_shared<GUI::Window>();
    win->setTitle("Dialog title");
    win->onClose(message::CANCEL);

    auto grid=GUI::Grid::rows(1); // 1 item per row, i.e. a single column
    win->setContent(grid);

    // And so on
}
```
`GUI::Key` contains most common keys that might be used. Modifier keys can be added with `+`, and numerals are accepted after modifiers as number keys. For example, `F`, `Alt+1`, and `Ctrl+Shift+PgDn` will all work as expected. The one exception is number keys with no modifier; you have to use either `3_key` or `GUI::ShortcutKey(3)` rather than just `3`.

Messages sent from the dialog are processed by the `handleMessage` function. They're sent in the form of `GUI::DialogMessage<message>`, which has three fields:
`message`, the message itself; `arg`, an argument that can be interpreted by casting it to the appropriate type; and `sender`, a pointer to the widget that sent the event. `handleMessage` should return `true` if the dialog should be closed and `false` if not.

The function for the dialog above might look like this:
```
bool WhateverDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
    switch(msg.message)
    {
    case message::SET_VALUE:
        // A numeric TextField sends its argument as an int32_t;
        // this will be implicitly cast.
        value=msg.argument;
        warningLabel->setVisible(value<0);
        return false;

    case message::HELP:
        showHelpMessage();
        return false;

    case message::OK:
        setValue(value);
        return true;

    case message::CANCEL:
        return true;
    }
	return false;
}
```

A useful technique is to take a callback `std::function` as an argument to the dialog's constructor and call it to apply changes when it's closed.

## Showing a dialog

All that needs to be done to show a dialog is to construct it and call its `show` function. These can be combined into a single statement.
```
WhateverDialog(currentValue,
    [&theValue](auto newValue)
    {
        theValue=newValue;
    }
).show();
```
