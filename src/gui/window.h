#ifndef ZC_GUI_WINDOW_H
#define ZC_GUI_WINDOW_H

#include "gui/dialog_ref.h"
#include "gui/top_level.h"
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

	/* Sets the window's title. */
	void setHelp(std::string newHelp);

	/* Sets the widget that will appear in the window. */
	void setContent(std::shared_ptr<Widget> newContent) noexcept;
	
	/* Sets if the dialog needs a d_vsync_proc or not */
	void setVSync(bool useVSync)
	{
		use_vsync = useVSync;
	}
	
	template<typename T>
	RequireMessage<T> onClose(T m)
	{
		closeMessage = static_cast<int32_t>(m);
		onKey(Key::Esc, m);
	}
	
	void setOnTick(std::function<int32_t()> newOnTick);
	void setOnLoad(std::function<void()> newOnLoad);
	
	void load();
private:
	std::shared_ptr<Widget> content;
	std::string title;
	DialogRef alDialog;
	int32_t closeMessage;
	bool use_vsync;
	std::function<int32_t()> onTick;
	std::function<void()> onLoad;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
