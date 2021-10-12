#ifndef ZC_GUI_SELTILE_SWATCH_H
#define ZC_GUI_SELTILE_SWATCH_H

#include "widget.h"
#include "dialog_ref.h"

namespace GUI
{

class SelTileSwatch: public Widget
{
public:
	SelTileSwatch();

	void setTile(int value);
	void setCSet(int value);

	int getTile();
	int getCSet();

	void setOnSelectFunc(std::function<void(int,int)> newOnSelect)
	{
		onSelectFunc = newOnSelect;
	}
	
	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int>(m);
	}
protected:
	int message;
private:
	int tile, cset;
	DialogRef alDialog;
	std::function<void(int,int)> onSelectFunc;
	
	void applyVisibility(bool visible) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	int onEvent(int event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
