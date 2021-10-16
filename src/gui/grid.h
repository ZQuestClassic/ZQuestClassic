#ifndef ZC_GUI_GRID_H
#define ZC_GUI_GRID_H

#include "widget.h"
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace GUI
{

class Grid: public Widget
{
public:
	enum class type { ROWS, COLUMNS };

	Grid(type growthType, size_t growthLimit);

	/* Create a new grid that fills left-to-right, then top-to-bottom. */
	static inline std::shared_ptr<Grid> rows(size_t itemsPerRow)
	{
		return std::make_shared<Grid>(type::ROWS, itemsPerRow);
	}

	/* Create a new grid that fills top-to-bottom, then left-to-right. */
	inline static std::shared_ptr<Grid> columns(size_t itemsPerCol)
	{
		return std::make_shared<Grid>(type::COLUMNS, itemsPerCol);
	}

	/* Set the space between rows. */
	inline void setRowSpacing(Size size) noexcept
	{
		rowSpacing = size.resolve();
	}

	/* Set the space between columns. */
	inline void setColumnSpacing(Size size) noexcept
	{
		colSpacing = size.resolve();
	}

	/* Set the space between rows and columns to the same value. */
	inline void setSpacing(Size size) noexcept
	{
		rowSpacing = size.resolve();
		colSpacing = size.resolve();
	}

	/* Add a widget at the next position in the grid. */
	inline void add(std::shared_ptr<Widget> child)
	{
		children.emplace_back(std::move(child));
	}

	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
private:
	std::vector<std::shared_ptr<Widget>> children;
	std::vector<int32_t> rowWidths, colWidths, rowHeights, colHeights;
	uint16_t rowSpacing, colSpacing;
	type growthType;
	size_t growthLimit;
};

}

#endif
