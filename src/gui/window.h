#ifndef ZC_GUI_WINDOW_H
#define ZC_GUI_WINDOW_H

#include "dialog_ref.h"
#include "top_level.h"
#include <memory>
#include <string>

namespace GUI
{

class Window: public TopLevelWidget
{
public:
	Window();

	/* Sets the window's title. */
	void setTitle(std::string newTitle);

	/* Sets the widget that will appear in the window. */
	void setContent(std::shared_ptr<Widget> newContent) noexcept;

	template<typename T>
	RequireMessage<T> onClose(T m)
	{
		closeMessage = static_cast<int>(m);
	}

private:
	std::shared_ptr<Widget> content;
	std::string title;
	DialogRef alDialog;
	int closeMessage;

	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
