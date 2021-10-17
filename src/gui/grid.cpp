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
	
	std::vector<int32_t> tempRowWidths, tempRowHeights;
	std::vector<int32_t> tempColWidths, tempColHeights;
	// Get the size of each row (first pass)
	for(size_t row = 0; row < numRows; ++row)
	{
		int32_t total = totalColSpacing, max = 0;
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			auto child = children[index];
			child->calculateSize();
			if(!child->getForceFitWid())
				total += child->getTotalWidth();
			
			if(!child->getForceFitHei())
				max = std::max(max, child->getTotalHeight());
		}
		tempRowWidths.push_back(total);
		tempRowHeights.push_back(max);
	}

	// Get the size of each column (first pass)
	for(size_t col = 0; col < numCols; ++col)
	{
		int32_t total = totalRowSpacing, max = 0;
		for(size_t row = 0; row < numRows; ++row)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			auto child = children[index];
			
			if(!child->getForceFitWid())
				max = std::max(max, child->getTotalWidth());
			
			if(!child->getForceFitHei())
				total += child->getTotalHeight();
		}
		tempColWidths.push_back(max);
		tempColHeights.push_back(total);
	}

	// Get the size of each row (second pass)
	for(size_t row = 0; row < numRows; ++row)
	{
		int32_t total = totalColSpacing, max = 0;
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			auto child = children[index];
			
			if(child->getForceFitWid())
				total += tempColWidths.at(col);
			else
				total += child->getTotalWidth();
			
			if(child->getForceFitHei())
				max = std::max(max, tempRowHeights.at(row));
			else
				max = std::max(max, child->getTotalHeight());
		}
		rowWidths.push_back(total);
		rowHeights.push_back(max);
	}
	
	// Get the size of each column (second pass)
	for(size_t col = 0; col < numCols; ++col)
	{
		int32_t total = totalRowSpacing, max = 0;
		for(size_t row = 0; row < numRows; ++row)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			auto child = children[index];
			
			if(child->getForceFitWid())
				max = std::max(max, tempColWidths.at(col));
			else
				max = std::max(max, child->getTotalWidth());
			
			if(child->getForceFitHei())
				total += tempRowHeights.at(row);
			else
				total += child->getTotalHeight();
		}
		colWidths.push_back(max);
		colHeights.push_back(total);
	}

	// Set the width to the longest row's width or the total column width,
	// whichever is greater.
	int32_t prefW = 0;
	for(auto& cw: colWidths)
		prefW += cw;
	for(auto& rw: rowWidths)
	{
		if(rw > prefW)
			prefW = rw;
	}
	setPreferredWidth(Size::pixels(prefW));

	// Similar deal for height.
	int32_t prefH = 0;
	for(auto& rh: rowHeights)
		prefH += rh;
	for(auto& ch: colHeights)
	{
		if(ch > prefH)
			prefH = ch;
	}
	setPreferredHeight(Size::pixels(prefH));
}


void Grid::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
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

	int32_t cy = y;
	for(size_t row = 0; row < numRows; ++row)
	{
		int32_t cx = x;
		int32_t c_hei = rowHeights[row];
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(index >= children.size())
				break;

			int32_t c_wid = colWidths[col];
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
