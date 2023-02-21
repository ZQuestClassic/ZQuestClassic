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
	{
		if(child.second)
			child.second->setExposed(visible);
	}
}

void Grid::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	for(auto& child: children)
	{
		if(child.second)
			child.second->setDisabled(dis);
	}
}

void Grid::add(std::shared_ptr<Widget> child)
{
	for(size_t q = 0;;++q)
	{
		if(usedIndexes[q]) continue;
		uint8_t mainspan, altspan;
		if(growthType == type::ROWS)
		{
			mainspan = child->getColSpan();
			altspan = child->getRowSpan();
		}
		else
		{
			mainspan = child->getRowSpan();
			altspan = child->getColSpan();
		}
		children[q] = std::move(child);
		
		for(size_t altind = 0; altind < altspan; ++altind)
		{
			for(size_t ind = 0; ind < mainspan && ((q+ind)/growthLimit == (q/growthLimit)); ++ind)
			{
				size_t calcIndex = q+ind+(altind*growthLimit);
				usedIndexes[calcIndex] = true;
				if(calcIndex!=q) children[calcIndex] = nullptr;
			}
		}
		return;
	}
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
		
		numRows = 1+(maxChildIndex() / growthLimit);
		numCols = growthLimit;
	}
	else
	{
		numRows = growthLimit;
		numCols = 1+(maxChildIndex() / growthLimit);
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
			if(!children[index])
				continue;

			auto& child = children[index];
			child->calculateSize();
			if(!child->getForceFitWid())
				total += child->getTotalWidth();
			else
			{
				auto mw = child->getMinWidth();
				if(mw > -1) total += mw;
			}
			
			if(child->getRowSpan() == 1)
			{
				if(!child->getForceFitHei())
					max = std::max(max, child->getTotalHeight());
				else
				{
					auto mh = child->getMinHeight();
					if(mh > -1) max = std::max(max, mh);
				}
			}
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
			if(!children[index])
				continue;

			auto& child = children[index];
			
			if(child->getColSpan() == 1)
			{
				if(!child->getForceFitWid())
					max = std::max(max, child->getTotalWidth());
				else
				{
					auto mw = child->getMinWidth();
					if(mw > -1) max = std::max(max, mw);
				}
			}
			
			if(!child->getForceFitHei())
				total += child->getTotalHeight();
			else
			{
				auto mh = child->getMinHeight();
				if(mh > -1) total += mh;
			}
		}
		tempColWidths.push_back(max);
		tempColHeights.push_back(total);
	}

	// Get the size of each row (second pass)
	for(size_t row = 0; row < numRows; ++row)
	{
		int32_t total = totalColSpacing, max = tempRowHeights.at(row);
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(!children[index])
				continue;

			auto& child = children[index];
			
			if(child->getForceFitWid())
			{
				for(size_t q = 0; q < child->getColSpan(); ++q)
					total += tempColWidths.at(col+q);
				total += (child->getColSpan() - 1) * colSpacing;
			}
			else
				total += child->getTotalWidth();
			
			if(child->getForceFitHei())
			{
				max = std::max(max, tempRowHeights.at(row));
			}
			else
			{
				if(child->getRowSpan() == 1)
					max = std::max(max, child->getTotalHeight());
				else
				{
					size_t brow = row+child->getRowSpan()-1;
					int32_t tmpheight = 0;
					for(size_t r = row; r < brow; ++r)
					{
						tmpheight += (tempRowHeights.at(r) + rowSpacing);
					}
					tempRowHeights[brow] = std::max(tempRowHeights.at(brow), child->getTotalHeight() - tmpheight);
				}
			}
		}
		rowWidths.push_back(total);
		rowHeights.push_back(max);
	}
	
	// Get the size of each column (second pass)
	for(size_t col = 0; col < numCols; ++col)
	{
		int32_t total = totalRowSpacing, max = tempColWidths.at(col);
		for(size_t row = 0; row < numRows; ++row)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(!children[index])
				continue;

			auto& child = children[index];
			
			if(child->getForceFitWid())
			{
				max = std::max(max, tempColWidths.at(col));
			}
			else
			{
				if(child->getColSpan() == 1)
					max = std::max(max, child->getTotalWidth());
				else
				{
					size_t rcol = col+child->getColSpan()-1;
					int32_t tmpwid = 0;
					for(size_t c = col; c < rcol; ++c)
					{
						tmpwid += (tempColWidths.at(c) + colSpacing);
					}
					tempColWidths[rcol] = std::max(tempColWidths.at(rcol), child->getTotalWidth() - tmpwid);
				}
			}
			
			if(child->getForceFitHei())
			{
				for(size_t q = 0; q < child->getRowSpan(); ++q)
					total += tempRowHeights.at(row+q);
				total += (child->getRowSpan() - 1) * rowSpacing;
			}
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
	Widget::calculateSize();
}


void Grid::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	// This currently just assumes there's enough space for everything to be
	// as big as it wants to be.
	size_t numRows, numCols;
	
	Widget::arrange(contX, contY, contW, contH);
	
	if(growthType == type::ROWS)
	{
		numRows = 1+(maxChildIndex() / growthLimit);
		numCols = growthLimit;
	}
	else
	{
		numRows = growthLimit;
		numCols = 1+(maxChildIndex() / growthLimit);
	}

	int32_t cy = y;
	for(size_t row = 0; row < numRows; ++row)
	{
		int32_t cx = x;
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = growthType == type::ROWS ?
				row*growthLimit+col :
				col*growthLimit+row;
			if(!children[index])
			{
				cx += colWidths[col]+colSpacing;
				continue;
			}
			auto& child = children[index];
			
			int32_t c_hei = rowSpacing * (child->getRowSpan()-1);
			int32_t c_wid = colSpacing * (child->getColSpan()-1);
			for(size_t q = 0; q < child->getRowSpan(); ++q)
			{
				c_hei += rowHeights[row+q];
			}
			for(size_t q = 0; q < child->getColSpan(); ++q)
			{
				c_wid += colWidths[col+q];
			}
			if(c_hei > (getHeight()-(cy-y)))
				c_hei = (getHeight()-(cy-y));
			if(c_wid > (getWidth()-(cx-x)))
				c_wid = (getWidth()-(cx-x));
			children[index]->arrange(cx, cy, c_wid, c_hei);
			cx += colWidths[col]+colSpacing;
		}
		cy += rowHeights[row]+rowSpacing;
	}
}

void Grid::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	for(auto& child: children)
	{
		if(child.second)
			child.second->realize(runner);
	}
}

}
