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
	void setTitle(std::string newTitle);
	void setContent(std::shared_ptr<Widget> newContent) noexcept;
	void setVisible(bool visible) override;

	template<typename T>
	void onClose(T m)
	{
		closeMessage=static_cast<int>(m);
	}

private:
	std::shared_ptr<Widget> content;
	std::string title;
	DialogRef alDialog;
	int closeMessage;

	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;
	int onEvent(int event, MessageDispatcher sendMessage) override;
};

}

#endif
