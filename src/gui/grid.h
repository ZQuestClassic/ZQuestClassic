#ifndef ZC_GUI_GRID_H
#define ZC_GUI_GRID_H

#include "widget.h"
#include <functional>
#include <memory>
#include <vector>

namespace GUI
{

class Grid: public Widget
{
public:
	Grid(): gridType(type::ROWS), size(2) {}
	static std::shared_ptr<Grid> rows(size_t itemsPerRow);
	static std::shared_ptr<Grid> columns(size_t itemsPerCol);
	void add(std::shared_ptr<Widget> child);
	void setVisible(bool visible) override;

private:
	enum class type { ROWS, COLUMNS };
	std::vector<std::shared_ptr<Widget>> children;
	std::vector<int> rowWidths, colWidths, rowHeights, colHeights;
	type gridType;
	size_t size;

	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
