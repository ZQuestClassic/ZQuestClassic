#ifndef ZC_GUI_SELTILE_SWATCH_H
#define ZC_GUI_SELTILE_SWATCH_H

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{

class SelTileSwatch: public Widget
{
public:
	SelTileSwatch();

	void setTile(int32_t value);
	void setCSet(int32_t value);
	void setFlip(int32_t value);
	void setShowFlip(bool val);
	void setShowVals(bool showVals);
	void setIsMini(bool val);
	void setMiniCrn(int32_t val) {mini_crn = vbound(val,0,3);}
	void setTileWid(int32_t val) {tw = std::max(1,val);}
	void setTileHei(int32_t val) {th = std::max(1,val);}
	
	void click();

	int32_t getTile();
	int32_t getCSet();
	int32_t getFlip();
	bool getIsMini() const {return isMini;}
	int32_t getMiniCrn() const {return mini_crn;}
	int32_t getTileWid() const {return tw;}
	int32_t getTileHei() const {return th;}

	void setOnSelectFunc(std::function<void(int32_t,int32_t,int32_t,int32_t)> newOnSelect)
	{
		onSelectFunc = newOnSelect;
	}
	
	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}
	
	int32_t sel_color;
	void calc_selcolor();
protected:
	int32_t message;
private:
	int32_t tile, cset, flip;
	int32_t tw, th;
	int32_t mini_crn;
	bool showFlip;
	bool showsVals;
	bool isMini;
	DialogRef alDialog;
	std::function<void(int32_t,int32_t,int32_t,int32_t)> onSelectFunc;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
