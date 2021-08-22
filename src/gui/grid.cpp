#include "grid.h"
#include <algorithm>

using std::shared_ptr;

namespace gui
{

shared_ptr<Grid> Grid::rows(size_t itemsPerRow)
{
    shared_ptr<Grid> g=std::make_shared<Grid>();
    g->gridType=Type::rows;
    g->size=itemsPerRow;
    return g;
}

shared_ptr<Grid> Grid::columns(size_t itemsPerCol)
{
    shared_ptr<Grid> g=std::make_shared<Grid>();
    g->gridType=Type::columns;
    g->size=itemsPerCol;
    return g;
}

void Grid::add(shared_ptr<Widget> child)
{
    children.push_back(child);
}

void Grid::calculateSize()
{
    // This needs a lot of work, but it'll do for now.
    rowWidths.clear();
    colWidths.clear();
    rowHeights.clear();
    colHeights.clear();
    size_t numRows, numCols;

    if(gridType==Type::rows)
    {
        numRows=(children.size()+size-1)/size; // +size-1 to round up
        numCols=size;
    }
    else
    {
        numRows=size;
        numCols=(children.size()+size-1)/size;
    }

    // Get the size of each row
    for(size_t row=0; row<numRows; row++)
    {
        int total=0, max=0;
        for(size_t col=0; col<numCols; col++)
        {
            size_t index=gridType==Type::rows ?
                row*size+col :
                col*size+row;
            if(index>=children.size())
                break;

            auto child=children[index];
            child->calculateSize();
            total+=child->getTotalWidth();
            max=std::max(max, child->getTotalHeight());
        }
        rowWidths.push_back(total);
        rowHeights.push_back(max);
    }

    // Get the size of each column
    for(size_t col=0; col<numCols; col++)
    {
        int total=0, max=0;
        for(size_t row=0; row<numRows; row++)
        {
            size_t index=gridType==Type::rows ?
                row*size+col :
                col*size+row;
            if(index>=children.size())
                break;

            auto child=children[index];
            max=std::max(max, child->getTotalWidth());
            total+=child->getTotalHeight();
        }
        colWidths.push_back(max);
        colHeights.push_back(total);
    }

    // Set the width to the longest row's width or the total column width,
    // whichever is greater.

    width=0;
    for(auto& cw: colWidths)
        width+=cw;
    for(auto& rw: rowWidths)
    {
        if(rw>width)
            width=rw;
    }

    // Similar deal for height.
    height=0;
    for(auto& rh: rowHeights)
        height+=rh;
    for(auto& ch: colHeights)
    {
        if(ch>height)
            height=ch;
    }
}


void Grid::arrange(int contX, int contY, int contW, int contH)
{
    // This currently just assumes there's enough space for everything to be
    // as big as it wants to be.
    size_t numRows, numCols;

    if(gridType==Type::rows)
    {
        numRows=(children.size()+size-1)/size; // +size-1 to round up
        numCols=size;
    }
    else
    {
        numRows=size;
        numCols=(children.size()+size-1)/size;
    }

    int y=contY;
    for(size_t row=0; row<numRows; row++)
    {
        int x=contX;
        int height=rowHeights[row];
        for(size_t col=0; col<numCols; col++)
        {
            size_t index=gridType==Type::rows ?
                row*size+col :
                col*size+row;
            if(index>=children.size())
                break;

            int width=colWidths[col];
            children[index]->arrange(x, y, width, height);
            x+=width;
        }
        y+=height;
    }

    Widget::arrange(contX, contY, contW, contH);
}

void Grid::realize(DialogRunner& runner)
{
    for(auto& child: children)
        child->realize(runner);
}

}
