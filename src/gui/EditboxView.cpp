#include <assert.h>

#include "gui/EditboxNew.h"
#include "base/zc_alleg.h"
#include "gui/jwin.h"
#include "base/fonts.h"
#include <map>
#include <stdio.h>
#include <sstream>

extern int32_t scheme[];

extern volatile int32_t myvsync;
void update_hw_screen(bool force);

extern bool is_editor();

void EditboxView::update()
{
	assert(model);
	layoutPage();
}

void EditboxView::initialize(EditboxModel *new_model)
{
	model = new_model;
	//add a "ghost newline" to the buffer if necessary
	
	string nl = "";
	Unicode::insertAtIndex(nl,'\n',0);
	uint32_t nlsize = (uint32_t)nl.size();
	
	if(model->getBuffer().size() < nlsize || model->getBuffer().substr(model->getBuffer().size()-nlsize,nlsize) != nl)
		model->getBuffer() += nl;
	
	dbuf = create_bitmap_ex(8, host->w, host->h);
	init();
	update();
}

void EditboxView::draw()
{
	assert(model);
}

EditboxView::~EditboxView()
{
	destroy_bitmap(dbuf);
}

///////////////////////////////////////////////////////////////////////////////////////

void EditboxView::lineUp()
{
	CursorPos cp = model->findCursor();
	list<LineData>::reverse_iterator it = list<LineData>::reverse_iterator(cp.it);
	
	if(it != model->getLines().rend())
	{
		int32_t startindex = model->getCursor().getPosition()-it->numchars-cp.index;
		int32_t index = Unicode::getIndexOfWidth(it->line, model->getCursor().getPreferredX(), textfont);
		model->getCursor().updateCursor(startindex+index);
	}
}

void EditboxView::lineDown()
{
	CursorPos cp = model->findCursor();
	list<LineData>::iterator it = cp.it;
	it++;
	int32_t startindex = model->getCursor().getPosition()+cp.it->numchars-cp.index;
	
	if(it != model->getLines().end())
	{
		int32_t index = Unicode::getIndexOfWidth(it->line, model->getCursor().getPreferredX(), textfont);
		model->getCursor().updateCursor(startindex+index);
	}
}

void EditboxView::lineHome(bool ctrl)
{
	if(ctrl)
	{
		model->getCursor().updateCursor(0);
		model->getCursor().setPreferredX();
		return;
	}
	CursorPos cp = model->findCursor();
	int32_t newindex = model->getCursor().getPosition()-cp.index;
	model->getCursor().updateCursor(newindex);
	model->getCursor().setPreferredX();
}

void EditboxView::lineEnd(bool ctrl)
{
	if(ctrl)
	{
		model->getCursor().updateCursor(model->getBuffer().size()-1);
		model->getCursor().setPreferredX();
		return;
	}
	CursorPos cp = model->findCursor();
	int32_t newindex = model->getCursor().getPosition()-cp.index+cp.it->numchars-1;
	model->getCursor().updateCursor(newindex);
	model->getCursor().setPreferredX();
}

void EditboxView::pageDown()
{
	int32_t textheight = text_height(textfont);
	int32_t height = getAreaHeight();
	int32_t numlines = height/textheight;
	
	for(int32_t i=0; i<int32_t(numlines); i++)
	{
		lineDown();
	}
}

void EditboxView::pageUp()
{
	int32_t textheight = text_height(textfont);
	int32_t height = getAreaHeight();
	int32_t numlines = height/textheight;
	
	for(int32_t i=0; i<numlines; i++)
	{
		lineUp();
	}
}

void EditboxView::invertRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	//I don't feel like dicking around with colormaps right now
	//this method is SLOOOW, big efficiency opportunity here
	static std::map<int32_t, int32_t> invmap;
	RGB color;
	//don't wast time drawing in stupid places
	x1 = zc_max(x1, 0);
	x1 = zc_min(x1, host->w);
	x2 = zc_max(x2,0);
	x2 = zc_min(x2, host->w);
	y1 = zc_max(y1, 0);
	y1 = zc_min(y1, host->h);
	y2 = zc_max(y2, 0);
	y2 = zc_min(y2, host->h);
	
	for(int32_t i=x1; i<x2; i++)
	{
		for(int32_t j=y1; j<y2; j++)
		{
			int32_t c = getpixel(dbuf, i,j);
			
			if(c != -1)
			{
				std::map<int32_t, int32_t>::iterator it = invmap.find(c);
				int32_t invcolor;
				
				if(it == invmap.end())
				{
					get_color(c, &color);
					uint8_t r = 4*(((~color.r)&0x3F)+1)-1;
					uint8_t g = 4*(((~color.g)&0x3F)+1)-1;
					uint8_t b = 4*(((~color.b)&0x3F)+1)-1;
					invcolor = makecol(r,g,b);
					invmap[c] = invcolor;
				}
				else
					invcolor = it->second;
				
				putpixel(dbuf, i,j,invcolor);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////

void BasicEditboxView::init()
{
	area_xstart = host->x+2;
	area_ystart = host->y+2;
	area_width = zc_max(0, host->w-20); //scrollbar
	area_height = host->h-4;
	view_width = area_width;
	
	//make the initial lines
	model->makeLines(model->getLines(), model->getBuffer());
}

void BasicEditboxView::ensureCursorOnScreen()
{
	CursorPos cp = model->findCursor();
	int32_t textheight = text_height(textfont);
	int32_t cystart = cp.lineno*textheight;
	int32_t cyend	= cystart+textheight;
	view_y = zc_min(view_y, cystart);
	view_y = zc_max(view_y, cyend-area_height);
	view_x = zc_min(view_x, cp.x);
	view_x = zc_max(view_x, cp.x-area_width);
	//enforce hard limits
	enforceHardLimits();
}

void BasicEditboxView::enforceHardLimits()
{
	int32_t textheight = text_height(textfont);
	int32_t ymost = zc_max(area_height, (int32_t)model->getLines().size()*textheight);
	view_y = zc_max(view_y, 0);
	view_y = zc_min(view_y, ymost-area_height);
	int32_t xmost = zc_max(area_width, view_width);
	view_x = zc_max(view_x, 0);
	view_x = zc_min(view_x, xmost-area_width);
}

BasicEditboxView::~BasicEditboxView()
{
	for(list<LineData>::iterator it = model->getLines().begin(); it != model->getLines().end(); it++)
	{
		destroy_bitmap(it->strip);
	}
	
	model->getLines().clear();
}

CharPos BasicEditboxView::findCharacter(int32_t x, int32_t y)
{
	int32_t absolutey = y-area_ystart+view_y;
	int32_t textheight = text_height(textfont);
	int32_t lineno = absolutey/textheight;
	lineno = zc_max(lineno, 0);
	lineno = zc_min(lineno, (int32_t)(model->getLines().size())-1);
	int32_t totalindex = 0;
	//NOTE: future optimization opportunity
	list<LineData>::iterator it = model->getLines().begin();
	
	for(int32_t i=0; i<lineno; i++)
	{
		totalindex += it->numchars;
		it++;
	}
	
	CharPos rval;
	rval.it = it;
	rval.lineIndex = Unicode::getIndexOfWidth(it->line, -area_xstart+x+view_x, textfont);
	rval.totalIndex = totalindex + rval.lineIndex;
	return rval;
}

void BasicEditboxView::scrollDown()
{
	int32_t textheight = text_height(textfont);
	view_y += textheight;
	enforceHardLimits();
}

void BasicEditboxView::scrollUp()
{
	int32_t textheight = text_height(textfont);
	view_y -= textheight;
	enforceHardLimits();
}

void BasicEditboxView::scrollLeft()
{
	view_x -= 4;
	enforceHardLimits();
}

void BasicEditboxView::scrollRight()
{
	view_x += 4;
	enforceHardLimits();
}

void BasicEditboxView::draw()
{
	rectfill(dbuf, 0, 0, host->w, host->h, bgcolor);
	set_clip_rect(dbuf, area_xstart-host->x, area_ystart-host->y, area_xstart-host->x+area_width-1, area_ystart-host->y+area_height-1);
	
	int32_t textheight = text_height(textfont);
	int32_t y = -view_y;
	
	for(list<LineData>::iterator it = model->getLines().begin(); it != model->getLines().end(); it++)
	{
		if(y >= area_ystart-host->y-textheight && y <= area_ystart+host->y + area_height)
			blit((*it).strip, dbuf, 0, 0, area_xstart-host->x-view_x, area_ystart-host->y+y, view_width, textheight);
		
		y+=textheight;
	}
	
	set_clip_rect(dbuf, 0,0,host->w,host->h);
	
	//draw cursor
	if(model->getCursor().isVisible())
	{
		CursorPos cp = model->findCursor();
		//int32_t textheight = text_height(textfont);
		int32_t cursory = cp.lineno*text_height(textfont);
		//GAH, too many damn coordinate offsets :-/
		vline(dbuf, area_xstart-host->x+cp.x-view_x-1, area_ystart-host->y+cursory-view_y-1, area_ystart-host->y+cursory-view_y+textheight, fgcolor);
	}
	
	//draw selection
	set_clip_rect(dbuf, area_xstart-host->x, area_ystart-host->y, area_xstart-host->x+area_width-1, area_ystart-host->y+area_height-1);
	
	if(model->getSelection().hasSelection())
	{
		pair<int32_t, int32_t> selection = model->getSelection().getSelection();
		CursorPos selstart = model->findIndex(selection.first);
		CursorPos selend = model->findIndex(selection.second);
		
		if(selstart.lineno == selend.lineno)
		{
			//invert the selection rectangle
			int32_t starty = area_ystart-host->y-view_y+selstart.lineno*textheight;
			int32_t startx = area_xstart-host->x+selstart.x-view_x;
			int32_t endx = area_xstart-host->x+selend.x-view_x;
			invertRectangle(startx, starty, endx, starty+textheight);
		}
		else
		{
			//do the starting line
			int32_t starty = area_ystart-host->y-view_y + selstart.lineno*textheight;
			int32_t startx = area_xstart-host->x+selstart.x-view_x;
			int32_t endx;
			
			if(hstyle == HSTYLE_EOLINE)
			{
				endx= area_xstart-host->x+area_width-view_x;
			}
			else
			{
				endx = area_xstart-host->x+selstart.it->strip->w-view_x;
			}
			
			invertRectangle(startx,starty,endx,starty+textheight);
			//do intermediate lines
			list<LineData>::iterator it = selstart.it;
			it++;
			
			for(int32_t line = selstart.lineno+1; line < selend.lineno; line++,it++)
			{
				int32_t endx2;
				
				if(hstyle == HSTYLE_EOLINE)
				{
					endx2=area_xstart-host->x+area_width-view_x;
				}
				else
				{
					endx2 = area_xstart-host->x+it->strip->w-view_x;
				}
				
				invertRectangle(area_xstart-host->x-view_x, area_ystart-host->y-view_y+line*textheight, endx2, area_ystart-host->y-view_y+(line+1)*textheight);
			}
			
			//do the last line
			endx = area_xstart-host->x+selend.x-view_x;
			invertRectangle(area_xstart-host->x-view_x,area_ystart-host->y-view_y+selend.lineno*textheight, endx, area_ystart-host->y-view_y+(selend.lineno+1)*textheight);
		}
	}
	
	set_clip_rect(dbuf, 0,0,host->w,host->h);
	drawExtraComponents();
	blit(dbuf, screen, 0, 0, host->x, host->y,host->w, host->h);
	set_clip_rect(screen, 0, 0,screen->w,screen->h);
	
	update_hw_screen(false);
}

bool BasicEditboxView::mouseClick(int32_t x, int32_t y)
{
	//set the cursor
	CharPos cp = findCharacter(x,y);
	model->getCursor().updateCursor(cp.totalIndex);
	model->getCursor().setPreferredX();
	model->getSelection().restartSelection(model->getCursor());
	return true;
}

bool BasicEditboxView::mouseDrag(int32_t x, int32_t y)
{
	int32_t textheight = text_height(textfont);
	
	if(model->getSelection().isSelecting())
	{
		pair<int32_t, int32_t> oldsel = model->getSelection().getSelection();
		CharPos cp = findCharacter(x,y);
		model->getCursor().updateCursor(cp.totalIndex);
		model->getSelection().adjustSelection(model->getCursor());
		
		if(y < area_ystart)
		{
			view_y = zc_max(0, view_y-1);
		}
		
		if(y > area_ystart+area_height)
		{
			int32_t ymost = zc_max(area_height, (int32_t)model->getLines().size()*textheight);
			view_y = zc_min(ymost-area_height, view_y+1);
		}
		
		if(x < area_xstart)
			view_x = zc_max(0, view_x-1);
			
		if(x > area_xstart+area_width)
		{
			int32_t xmost = zc_max(area_width, view_width);
			view_x = zc_min(xmost-area_width, view_x+1);
		}
		
		if(oldsel != model->getSelection().getSelection())
		{
			return true;
		}
	}
	
	return false;
}

bool BasicEditboxView::mouseRelease(int32_t x, int32_t y)
{
	x=x;
	y=y; //these are here to bypass compiler warnings about unused arguments
	model->getSelection().doneSelection();
	return false;
}

void BasicEditboxView::createStripBitmap(list<LineData>::iterator it, int32_t width)
{
	//now create the bitmap
	int32_t textheight = text_height(textfont);
	
	if(it->strip)
		destroy_bitmap(it->strip);
	
	it->strip = create_bitmap_ex(8,width,textheight);
	rectfill(it->strip, 0,0,width, textheight, bgcolor);
	Unicode::textout_ex_nonstupid(it->strip, textfont, (*it).line, 0, 0, fgcolor, bgcolor);
}

////////////////////////////////////////////////////////////////////////////////////////

void EditboxVScrollView::init()
{
	BasicEditboxView::init();
	toparrow_x = host->x+host->w-16;
	toparrow_y = host->y;
	toparrow_state=0;
	bottomarrow_x = host->x+host->w-16;
	bottomarrow_y = host->y+host->h-16;
	bottomarrow_state = 0;
	barstate = 0;
}

void EditboxVScrollView::drawExtraComponents()
{
	int32_t textheight = text_height(textfont);
	//draw the scrollbar
	draw_arrow_button(dbuf, toparrow_x-host->x, toparrow_y-host->y, 16, 16, true, toparrow_state*3);
	draw_arrow_button(dbuf, bottomarrow_x-host->x, bottomarrow_y-host->y, 16, 16, false, bottomarrow_state*3);
	
	if(!sbarpattern)
	{
		sbarpattern = create_bitmap_ex(bitmap_color_depth(screen),2,2);
		putpixel(sbarpattern, 0, 1, scheme[jcLIGHT]);
		putpixel(sbarpattern, 0, 1, scheme[jcLIGHT]);
		putpixel(sbarpattern, 0, 1, scheme[jcLIGHT]);
		putpixel(sbarpattern, 0, 1, scheme[jcLIGHT]);
	}
	
	drawing_mode(DRAW_MODE_COPY_PATTERN, sbarpattern, 0, 0);
	int32_t barstart = toparrow_y + 16 - host->y;
	int32_t barend = bottomarrow_y - host->y-1;
	
	if(barstart < barend)
		rectfill(dbuf, toparrow_x-host->x, barstart, toparrow_x-host->x+15, barend, 0);
	
	solid_mode();
	//compute the bar button, based on view_y
	int32_t totallen = (int32_t)model->getLines().size()*textheight;
	int32_t available = bottomarrow_y-(toparrow_y+16);
	
	if(available < 0)
	{
		baroff=barlen=0;
	}
	
	else
	{
		//area_height:totallen = barlen:available
		barlen = (available*area_height)/zc_max(totallen,area_height)+1;
		//clip to reasonable values
		barlen = zc_max(barlen, 8);
		baroff = zc_min(baroff, available-barlen);
		
		//view_y:(totallen-area_height) = baroff:(available-barlen)
		if(totallen <= area_height)
			baroff=0;
		else
			baroff = ((available-barlen)*view_y)/(totallen-area_height);
	}
	
	if(barlen > 0)
	{
		jwin_draw_button(dbuf, toparrow_x-host->x, toparrow_y+16-host->y+baroff, 16, barlen, false, 1);
	}
}

EditboxVScrollView::~EditboxVScrollView()
{
	destroy_bitmap(sbarpattern);
}

bool EditboxVScrollView::mouseClick(int32_t x, int32_t y)
{
	//check if in text area
	if(area_ystart <= y && y <= area_ystart+area_height)
	{
		if(area_xstart <= x && x <= area_xstart+area_width)
		{
			return BasicEditboxView::mouseClick(x,y);
		}
		
		if(toparrow_x <= x && x <= toparrow_x + 16)
		{
			// clicked on an arrow, or the slider
			if(barstate == 1)
			{
				//adjust
				int32_t deltay = barstarty-y;
				barstarty = y;
				//deltay:(available-barlen) = dealtaview_y:(totallen-area_height)
				int32_t available = bottomarrow_y-(toparrow_y+16);
				int32_t textheight = text_height(textfont);
				int32_t totallen = (int32_t)model->getLines().size()*textheight;
				
				if(available > barlen)
					view_y -= ((totallen-area_height)*deltay)/(available-barlen);
				
				enforceHardLimits();
				return true;
			}
			
			if(toparrow_y <= y && y <= toparrow_y + 16)
			{
				//clicked on top arrow
				scrollUp();
				toparrow_state = 1;
				return true;
			}
			
			if(bottomarrow_y <= y && y <= bottomarrow_y + 16)
			{
				scrollDown();
				bottomarrow_state = 1;
				return true;
			}
			else
			{
				//clicked the slider
				if(toparrow_y+16+baroff <= y && y <= toparrow_y+16+baroff+barlen)
				{
					//clicked the bar itself
					barstarty = y;
					barstate = 1;
					return true;
				}
				else
				{
					//"teleport"
					//adjust click by half of length of slider
					y -= toparrow_y+16+barlen/2;
					int32_t available = bottomarrow_y-(toparrow_y+16);
					int32_t textheight = text_height(textfont);
					int32_t totallen = (int32_t)model->getLines().size()*textheight;
					
					//y:(available-barlen)= view_y:(totallen-area_height)
					if(available <= barlen)
						view_y=0;
					else
						view_y = (y*(totallen-area_height))/(available-barlen);
					
					enforceHardLimits();
					return true;
				}
			}
		}
	}
	
	return mouseClickOther(x,y);
}

bool EditboxVScrollView::mouseDrag(int32_t x, int32_t  y)
{
	int32_t textheight;
	textheight = text_height(textfont);
	
	if(model->getSelection().isSelecting())
	{
		return BasicEditboxView::mouseDrag(x,y);
	}
	else
	{
		//maybe pressing arrow, or sliding?
		if(toparrow_state == 1)
		{
			scrollUp();
			return true;
		}
		
		if(bottomarrow_state == 1)
		{
			scrollDown();
			return true;
		}
		
		if(barstate == 1)
		{
			//fake a click
			//first, clip the coords
			int32_t fakex = toparrow_x+1;
			return mouseClick(fakex,y);
		}
		
		return mouseDragOther(x,y);
	}
}

bool EditboxVScrollView::mouseRelease(int32_t x, int32_t y)
{
	BasicEditboxView::mouseRelease(x,y);
	toparrow_state = 0;
	bottomarrow_state = 0;
	barstate = 0;
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
void EditboxWordWrapView::layoutPage()
{
	//check all lines
	for(list<LineData>::iterator it = model->getLines().begin(); it != model->getLines().end(); it++)
	{
		if(!it->dirtyflag)
			continue;
		
		int32_t numchars = (*it).numchars;
		string &s = (*it).line;
		//accumulate up until the maximum line width
		int32_t totalwidth=0;
		int32_t i;
		
		for(i=0; i<numchars; i++)
		{
			if(i==0)
			{
				//we must be more accepting of the first character/word since the box
				//might not be wide enough.
				int32_t c = Unicode::getCharAtIndex(s,0);
				
				if(c == ' ' || c == '\t' || c == '\n')
				{
					//whitespace
					//accept and continue
					totalwidth += Unicode::getCharWidth(c, textfont);
					continue;
				}
				else
				{
					//word
					pair<int32_t, int32_t> offandwidth = Unicode::munchWord(s,i,textfont);
					totalwidth += offandwidth.second;
					i += offandwidth.first-1;
					continue;
				}
			}
			else
			{
				int32_t c = Unicode::getCharAtIndex(s,i);
				
				if(c == ' ' || c == '\t' || c == '\n')
				{
					//whitespace
					totalwidth += Unicode::getCharWidth(c, textfont);
					
					if(totalwidth > area_width)
						break;
				}
				else
				{
					pair<int32_t, int32_t> offandwidth = Unicode::munchWord(s,i,textfont);
					totalwidth += offandwidth.second;
					
					if(totalwidth > area_width)
						break;
					
					i += offandwidth.first-1;
				}
			}
		}
		
		if(i < numchars)
		{
			//we have wrapped early.
			string newline = s.substr(i,s.size()-i);
			LineData newdata = {newline, numchars-i, true, true,NULL};
			list<LineData>::iterator it2 = it;
			it2++;
			model->getLines().insert(it2,newdata);
			(*it).line = s.substr(0,i);
			(*it).numchars  = i;
			(*it).newlineterminated = false;
		}
		
		totalwidth=0;
		//efficiency opportunity
		int32_t length = Unicode::getLength(it->line);
		
		for(int32_t j=0; j<length; j++)
		{
			totalwidth += Unicode::getCharWidth(Unicode::getCharAtIndex(it->line,j),textfont);
		}
		
		createStripBitmap(it, totalwidth);
		(*it).dirtyflag = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void EditboxNoWrapView::layoutPage()
{
	view_width = area_width;
	
	for(list<LineData>::iterator it = model->getLines().begin(); it != model->getLines().end(); it++)
	{
		int32_t length = Unicode::getLength(it->line);
		int32_t totalwidth=0;
		
		//efficiency opportunity
		for(int32_t i=0; i<length; i++)
		{
			totalwidth += Unicode::getCharWidth(Unicode::getCharAtIndex(it->line,i),textfont);
		}
		
		if(totalwidth > view_width)
			view_width=totalwidth;
		
		if(!it->dirtyflag)
			continue;
		
		createStripBitmap(it, totalwidth);
		(*it).dirtyflag=false;
	}
}

void EditboxNoWrapView::init()
{
	EditboxVScrollView::init();
	bottomarrow_y -= 16;
	area_height -=16;
	leftarrow_x = host->x;
	leftarrow_y = host->y+host->h-16;
	leftarrow_state=0;
	rightarrow_x = host->x+host->w-32;
	rightarrow_y = host->y+host->h-16;
	rightarrow_state = 0;
	hbarstate = 0;
}

void EditboxNoWrapView::drawExtraComponents()
{
	EditboxVScrollView::drawExtraComponents();
	int32_t textheight;
	textheight = text_height(textfont);
	//draw the scrollbar
	draw_arrow_button_horiz(dbuf, leftarrow_x-host->x, leftarrow_y-host->y, 16, 16, true, leftarrow_state*3);
	draw_arrow_button_horiz(dbuf, rightarrow_x-host->x, rightarrow_y-host->y, 16, 16, false, rightarrow_state*3);
	drawing_mode(DRAW_MODE_COPY_PATTERN, sbarpattern, 0, 0);
	int32_t hbarstart = leftarrow_x + 16 - host->x;
	int32_t hbarend = rightarrow_x - host->x-1;
	
	if(hbarstart < hbarend)
		rectfill(dbuf, hbarstart, leftarrow_y-host->y, hbarend, leftarrow_y-host->y+15, 0);
	
	solid_mode();
	//compute the bar button, based on view_y
	int32_t totallen = view_width;
	int32_t available = rightarrow_x-(leftarrow_x+16);
	
	if(available < 0)
	{
		hbaroff=hbarlen=0;
	}
	else
	{
		//area_width:totallen = barlen:available
		hbarlen = (available*area_width)/zc_max(totallen,area_width)+1;
		//clip to reasonable values
		hbarlen = zc_max(hbarlen, 8);
		hbaroff = zc_min(hbaroff, available-hbarlen);
		
		//view_x:(totallen-area_width) = baroff:(available-hbarlen)
		if(totallen <= area_width)
			hbaroff = 0;
		else
			hbaroff = ((available-hbarlen)*view_x)/(totallen-area_width);
	}
	
	if(hbarlen > 0)
	{
		jwin_draw_button(dbuf, leftarrow_x-host->x+hbaroff+16, leftarrow_y-host->y, hbarlen, 16, false, 1);
	}
}

bool EditboxNoWrapView::mouseRelease(int32_t x, int32_t y)
{
	leftarrow_state = 0;
	rightarrow_state = 0;
	hbarstate = 0;
	return EditboxVScrollView::mouseRelease(x,y);
}

bool EditboxNoWrapView::mouseDragOther(int32_t x, int32_t)
{
	//maybe pressing arrow, or sliding?
	if(leftarrow_state == 1)
	{
		scrollLeft();
		return true;
	}
	
	if(rightarrow_state == 1)
	{
		scrollRight();
		return true;
	}
	
	if(hbarstate == 1)
	{
		//fake a click
		//first, clip the coords
		int32_t fakey = leftarrow_y+1;
		return mouseClick(x,fakey);
	}
	
	return false;
}

bool EditboxNoWrapView::mouseClickOther(int32_t x, int32_t y)
{
	if(leftarrow_x <= x && x <= rightarrow_x+16)
	{
		if(leftarrow_y <= y && y <= leftarrow_y+16)
		{
			// clicked on an arrow, or the slider
			if(hbarstate == 1)
			{
				//adjust
				int32_t deltax = hbarstartx-x;
				hbarstartx = x;
				//deltax:(available-hbarlen) = dealtaview_x:(totallen-area_width)
				int32_t available = rightarrow_x-(leftarrow_x+16);
				int32_t totallen = view_width;
				
				if(available > hbarlen)
					view_x -= ((totallen-area_width)*deltax)/(available-hbarlen);
				
				enforceHardLimits();
				return true;
			}
			
			if(leftarrow_x <= x && x <= leftarrow_x + 16)
			{
				//clicked on left arrow
				scrollLeft();
				leftarrow_state = 1;
				return true;
			}
			
			if(rightarrow_x <= x && x <= rightarrow_x + 16)
			{
				scrollRight();
				rightarrow_state = 1;
				return true;
			}
			else
			{
				//clicked the slider
				if(leftarrow_x+16+hbaroff <= x && x <= leftarrow_x+16+hbaroff+hbarlen)
				{
					//clicked the bar itself
					hbarstartx = x;
					hbarstate = 1;
					return true;
				}
				else
				{
					//"teleport"
					//adjust click by half of length of slider
					x -= leftarrow_x+16+hbarlen/2;
					int32_t available = rightarrow_x-(leftarrow_x+16);
					int32_t totallen = view_width;
					
					//x:(available-hbarlen)= view_x:(totallen-area_width)
					if(available <= hbarlen)
						view_x=0;
					else
						view_x = (x*(totallen-area_width))/(available-hbarlen);
					
					enforceHardLimits();
					return true;
				}
			}
		}
	}
	
	return false;
}

void EditboxScriptView::init()
{
	EditboxNoWrapView::init();
	bottomarrow_y-=16;
	area_height-=16;
	leftarrow_y-=16;
	rightarrow_y-=16;
	linetext = create_bitmap_ex(8,host->w,16);
}

EditboxScriptView::~EditboxScriptView()
{
	if(linetext) destroy_bitmap(linetext);
}

void EditboxScriptView::drawExtraComponents()
{
	EditboxNoWrapView::drawExtraComponents();
	rectfill(linetext, 0,0,linetext->w, linetext->h, scheme[jcMEDLT]);
	CursorPos cp = model->findCursor();
	char temp[60];
	sprintf(temp, "Line %d", cp.lineno+1);
	//center text
	FONT* linefont = get_custom_font(CFONT_GUI);
	int32_t textheight = text_height(linefont);
	int32_t padding = 16-textheight;
	int32_t offset = padding/2;
	textout_ex(linetext, linefont, temp, 2,offset,fgcolor, -1);
	blit(linetext, dbuf, 0,0, 0, leftarrow_y-host->y+16,linetext->w,linetext->h);
}

