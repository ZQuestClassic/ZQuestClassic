#include "grid.h"
#include <algorithm>

using std::shared_ptr;

namespace GUI
{

Grid::Grid(type growthType, size_t growthLimit):
	rowSpacing(0), colSpacing(0),
	growthType(growthType),
	growthLimit(growthLimit)
{}

void Grid::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	for(auto& child: children)
		child->applyVisibility(visible);
}

void Grid::calculateSize()
{
	rowWidths.clear();
	colWidths.clear();
	rowHeights.clear();
	colHeights.clear();
	size_t numRows, numCols, totalRowSpacing, totalColSpacing;

	if(growthType == type::ROWS)
	{
		// +growthLimit-1 to round up
		numRows = (children.size()+growthLimit-1)/growthLimit;
		numCols = growthLimit;
	}
	else
	{
		numRows = growthLimit;
		numCols = (children.size()+growthLimit-1)/growthLimit;
	}

	totalRowSpacing = (numRows-1)*rowSpacing;
	totalColSpacing = (numCols-1)*colSpacing;

	// Get the size of each row
	for(size_t row = 0; row < numRows; ++row)
	{
		int total = totalColSpacing, max = 0;
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			auto child = children[index];
			child->calculateSize();
			total += child->getTotalWidth();
			max = std::max(max, child->getTotalHeight());
		}
		rowWidths.push_back(total);
		rowHeights.push_back(max);
	}

	// Get the size of each column
	for(size_t col = 0; col < numCols; ++col)
	{
		int total = totalRowSpacing, max = 0;
		for(size_t row = 0; row < numRows; ++row)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			auto child = children[index];
			max = std::max(max, child->getTotalWidth());
			total += child->getTotalHeight();
		}
		colWidths.push_back(max);
		colHeights.push_back(total);
	}

	// Set the width to the longest row's width or the total column width,
	// whichever is greater.
	int prefW = 0;
	for(auto& cw: colWidths)
		prefW += cw;
	for(auto& rw: rowWidths)
	{
		if(rw > prefW)
			prefW = rw;
	}
	setPreferredWidth(Size::pixels(prefW));

	// Similar deal for height.
	int prefH = 0;
	for(auto& rh: rowHeights)
		prefH += rh;
	for(auto& ch: colHeights)
	{
		if(ch > prefH)
			prefH = ch;
	}
	setPreferredHeight(Size::pixels(prefH));
}


void Grid::arrange(int contX, int contY, int contW, int contH)
{
	// This currently just assumes there's enough space for everything to be
	// as big as it wants to be.
	size_t numRows, numCols;
	
	Widget::arrange(contX, contY, contW, contH);
	
	if(growthType == type::ROWS)
	{
		// +growthLimit-1 to round up
		numRows = (children.size()+growthLimit-1)/growthLimit;
		numCols = growthLimit;
	}
	else
	{
		numRows = growthLimit;
		numCols = (children.size()+growthLimit-1)/growthLimit;
	}

	int cy = y;
	for(size_t row = 0; row < numRows; ++row)
	{
		int cx = x;
		int c_hei = rowHeights[row];
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			int c_wid = colWidths[col];
			if(c_hei > (getHeight()-(cy-y)))
				c_hei = (getHeight()-(cy-y));
			if(c_wid > (getWidth()-(cx-x)))
				c_wid = (getWidth()-(cx-x));
			children[index]->arrange(cx, cy, c_wid, c_hei);
			cx += c_wid+colSpacing;
		}
		cy += c_hei+rowSpacing;
	}
}

void Grid::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	for(auto& child: children)
		child->realize(runner);
}

}
