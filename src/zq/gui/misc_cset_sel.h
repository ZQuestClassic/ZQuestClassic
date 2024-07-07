#ifndef ZC_GUI_MISC_CSET_SEL_H_
#define ZC_GUI_MISC_CSET_SEL_H_

#include "gui/widget.h"
#include "gui/label.h"
#include "gui/grid.h"
#include "gui/drop_down_list.h"
#include <vector>
#include "gui/dialog_ref.h"

namespace GUI
{

class MiscCSetSel: public Widget
{
public:
	MiscCSetSel();
	
	void setC1(int32_t val);
	void setC2(int32_t val);
	int32_t getC1() const {return c1;}
	int32_t getC2() const {return c2;}
	void setOnUpdate(std::function<void(int32_t,int32_t)> newOnUpdate);

private:
	std::function<void(int32_t,int32_t)> onUpdate;
	
	std::shared_ptr<Grid> internal_grid;
	std::shared_ptr<Label> labels[2];
	std::shared_ptr<DropDownList> sel_list;
	std::shared_ptr<DropDownList> misc_sel_list;
	
	int32_t c1, c2;
	
	void doVis(int32_t _c1);
	void doUpdate(int32_t _c1, int32_t _c2);
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
