#ifndef ZC_GUI_MISC_COLOR_SEL_H_
#define ZC_GUI_MISC_COLOR_SEL_H_

#include "gui/widget.h"
#include "gui/label.h"
#include "gui/switcher.h"
#include "gui/grid.h"
#include "gui/drop_down_list.h"
#include "misc_color_row.h"
#include <vector>
#include "gui/dialog_ref.h"

namespace GUI
{

class MiscColorSel: public Widget
{
public:
	MiscColorSel();
	
	void setC1(int32_t val);
	void setC2(int32_t val);
	void setOnUpdate(std::function<void(int32_t,int32_t)> newOnUpdate);

private:
	std::function<void(int32_t,int32_t)> onUpdate;
	
	std::shared_ptr<Grid> internal_grid;
	std::shared_ptr<Label> labels[2];
	std::shared_ptr<DropDownList> sel_list;
	std::shared_ptr<Switcher> sw;
	std::shared_ptr<DropDownList> misc_sel_list;
	std::shared_ptr<MiscColorRow> mc_sys_row;
	std::shared_ptr<MiscColorRow> mc_cs_row;
	
	int32_t c1, c2;
	int32_t c2s[14];
	
	void doUpdate1(int32_t val, bool func = true);
	void doUpdate2(int32_t val, bool func = true);
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
