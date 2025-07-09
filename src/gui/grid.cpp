#include "gui/grid.h"
#include <algorithm>

using std::shared_ptr;

namespace GUI
{

Grid::Grid(type growthType, size_t size1, size_t size2):
	rowSpacing(0), colSpacing(0),
	growthType(growthType),
	size1(size1), size2(size2)
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
		size_t col_lim = size_t(-1), row_lim = size_t(-1);
		
		switch(growthType)
		{
			case type::ROWS:
				col_lim = size1;
				break;
			case type::COLUMNS:
				row_lim = size1;
				break;
			case type::ROWS_COLUMNS:
				row_lim = size2;
				break;
			case type::COLUMNS_ROWS:
				col_lim = size2;
				break;
		}
		
		for(size_t colind = 0; colind < child->getColSpan(); ++colind)
		{
			auto col = get_col(q)+colind;
			if(col >= col_lim)
				continue;
			for(size_t rowind = 0; rowind < child->getRowSpan(); ++rowind)
			{
				auto row = get_row(q)+rowind;
				if(row >= row_lim)
					continue;
				auto indx = get_index(row,col);
				usedIndexes[indx] = true;
				children[indx] = nullptr;
			}
		}
		//usedIndexes[q] = true;
		children[q] = std::move(child);
		return;
	}
}

void Grid::calculateSize()
{
	rowWidths.clear();
	colWidths.clear();
	rowHeights.clear();
	colHeights.clear();
	auto [numRows, numCols] = get_counts();
	size_t totalRowSpacing, totalColSpacing;

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
			size_t index = get_index(row,col);
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
			size_t index = get_index(row,col);
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
			size_t index = get_index(row,col);
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
			size_t index = get_index(row,col);
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
	auto [numRows, numCols] = get_counts();
	
	Widget::arrange(contX, contY, contW, contH);
	
	int32_t cy = y;
	for(size_t row = 0; row < numRows; ++row)
	{
		int32_t cx = x;
		for(size_t col = 0; col < numCols; ++col)
		{
			size_t index = get_index(row, col);
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


size_t Grid::get_index(size_t row, size_t col) const
{
	switch(growthType)
	{
		case type::ROWS:
			return row*size1+col;
		case type::COLUMNS:
			return col*size1+row;
		case type::ROWS_COLUMNS:
		{
			auto unit = col/size1;
			auto sub = (col % size1)+(row * size1);
			return unit*(size1*size2) + sub;
		}
		case type::COLUMNS_ROWS:
		{
			auto unit = row/size1;
			auto sub = (row % size1)+(col * size1);
			return unit*(size1*size2) + sub;
		}
		default:
			assert(false);
			return 0;
	}
}
size_t Grid::get_row(size_t index) const
{
	switch(growthType)
	{
		case type::ROWS:
			return index/size1;
		case type::COLUMNS:
			return index%size1;
		case type::ROWS_COLUMNS:
			return (index/size1)%size2;
		case type::COLUMNS_ROWS:
			return (index%size1) + ((index/(size1*size2))*size1);
		default:
			assert(false);
			return 0;
	}
}
size_t Grid::get_col(size_t index) const
{
	switch(growthType)
	{
		case type::ROWS:
			return index%size1;
		case type::COLUMNS:
			return index/size1;
		case type::ROWS_COLUMNS:
			return (index%size1) + ((index/(size1*size2))*size1);
		case type::COLUMNS_ROWS:
			return (index/size1)%size2;
		default:
			assert(false);
			return 0;
	}
}
std::pair<size_t,size_t> Grid::get_counts() const
{
	size_t numRows, numCols;
	switch(growthType)
	{
		case type::ROWS:
			numRows = 1+(maxChildIndex() / size1);
			numCols = size1;
			break;
		case type::COLUMNS:
			numRows = size1;
			numCols = 1+(maxChildIndex() / size1);
			break;
		case type::ROWS_COLUMNS:
			numRows = size2;
			numCols = size1*(1+(maxChildIndex() / (size1*size2)));
			break;
		case type::COLUMNS_ROWS:
			numRows = size1*(1+(maxChildIndex() / (size1*size2)));
			numCols = size2;
			break;
	}
	return std::make_pair(numRows, numCols);
}
std::map<size_t, std::shared_ptr<Widget>> Grid::get_children()
{
	return children;
}

}
