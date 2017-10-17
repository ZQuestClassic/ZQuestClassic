#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <assert.h>

#include "EditboxNew.h"
#include "zc_alleg.h"
#include "jwin.h"
#include "zdefs.h"
#include "editbox.h"
#include "gui.h"
#include <stdio.h>

extern FONT *lfont, *sfont3;
extern FONT *pfont;
extern bool is_large;
extern int d_timer_proc(int msg, DIALOG *d, int c);

static DIALOG help_dlg[] =
{
  { jwin_win_proc,        0,    0,   320,  240,    0,       vc(15),  0,       D_EXIT,     0,          0,        (void *) "ZQuest Help", NULL, NULL },
  { jwin_frame_proc,      4,   23,   312,  213,    0,       0,       0,       0,          FR_DEEP,    0,         NULL, NULL, NULL },
  { d_editbox_proc,       6,   25,   308,  209,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
  { d_keyboard_proc,      0,    0,     0,    0,    0,       0,       0,       0,          0,          KEY_ESC,  (void *) close_dlg, NULL, NULL },
  { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
  { NULL,                 0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL }
};

int Unicode::indexToOffset(string &s, int i)
{
	return uoffset(s.c_str(), i);
}

void Unicode::insertAtIndex(string &s, int c, int i)
{
	char temp[50];
	int bytes = usetc(temp, c);
	usetc(temp+bytes,0);
	int offset = indexToOffset(s,i);
	s = s.substr(0,offset) + temp + s.substr(offset, s.size()-offset);
}

void Unicode::extractRange(string &s, string &dest, int first, int last)
{
	int firstoffset = indexToOffset(s,first);
	int lastoffset = indexToOffset(s,last);
	dest = s.substr(firstoffset, lastoffset-firstoffset);
}

void Unicode::removeRange(string &s, int first, int last)
{
	int firstoffset = indexToOffset(s,first);
	int lastoffset = indexToOffset(s,last);
	s = s.substr(0,firstoffset) + s.substr(lastoffset, s.size()-lastoffset);
}

int Unicode::getCharAtIndex(string &s, int i)
{
	int offset = indexToOffset(s,i);
	return ugetc(s.c_str()+offset);
}

int Unicode::getCharWidth(int c, FONT *f)
{
	if(c == '\n')
		return  0;
	if(c == '\t')
		return TABSIZE*getCharWidth(' ', f);
	char temp[50];
	int width = usetc(temp, c);
	usetc(temp+width,0);
	return text_length(f,temp);
}

int Unicode::getCharWidth(const char *s, int offset)
{
	return uwidth(s+offset);
}

int Unicode::getCharAtOffset(const char *s, int offset)
{
	return ugetc(s+offset);
}

pair<int, int> Unicode::munchWord(string &s, int startoffset, FONT *f)
{
	unsigned int endoffset;
	for(endoffset = startoffset; endoffset < s.size(); endoffset++)
	{
		int c = Unicode::getCharAtIndex(s,endoffset);
		{
			if(c == ' ' || c == '\t')
				break;
		}
	}
	pair<int, int> rval;
	rval.first = endoffset-startoffset; //guaranteed >= 1
	rval.second = text_length(f,s.substr(startoffset, endoffset-startoffset).c_str());
	return rval;
}

void Unicode::textout_ex_nonstupid(BITMAP *bmp, FONT *f, string &s, int x, int y, int fg, int bg)
{
	unsigned int offset = 0;
	string newstring;
	const char *txt = s.c_str();
	while(offset < s.size())
	{
		int c = ugetc(txt+offset);

		int width = uwidth(txt);
		offset+=width;
		char temp[50];
		if(c == '\n')
			continue;
		if(c == '\t')
		{
			int temp_offset = 0;
			for(int i=0; i<TABSIZE; i++)
			{
				int temp_width = usetc(temp+temp_offset, ' ');
				temp_offset+=temp_width;
			}
			usetc(temp+temp_offset,0);
		}
		else
		{
			int temp_width = usetc(temp,c);
			usetc(temp+temp_width,0);
		}
		newstring += temp;
	}
	textout_ex(bmp, f, newstring.c_str(), x, y, fg, bg);
}

int Unicode::getIndexOfWidth(string &s, int x, FONT *f)
{
	int index = 0;
	unsigned int offset = 0;
	int pixwidth=0;
	int lastwidth=0;
	const char *buf = s.c_str();
	int c;
	for(index=0; ; index++)
	{
		int width = Unicode::getCharWidth(buf, offset);
		c = Unicode::getCharAtOffset(buf, offset);
		lastwidth = Unicode::getCharWidth(c,f);
		pixwidth += lastwidth;
		offset += width;
		if(offset == s.size() || c == '\n' || pixwidth > x)
			break;
	}
	if(offset == s.size() ||c == '\n')
		return index;
	//else try the next char, see if it's closer
	if(x-pixwidth+lastwidth < pixwidth - x)
		return index;
	return index+1;
}

int Unicode::getLength(string &s)
{
	int length=0;
	unsigned int offset=0;
	const char *buf = s.c_str();
	while(offset < s.length())
	{
		offset += Unicode::getCharWidth(buf,offset);
		length++;
	}
	return length;
}

bool TextSelection::hasSelection()
{
	return (start != end);
}

pair<int, int> TextSelection::getSelection()
{
	if(start < end)
		return pair<int, int>(start,end);
	return pair<int, int>(end, start);
}

void TextSelection::clearSelection()
{
	start=end=0;
	isselecting = false;
}

void TextSelection::restartSelection(EditboxCursor &cursor)
{
	isselecting = true;
	start=end=cursor.getPosition();
}

void TextSelection::adjustSelection(EditboxCursor &cursor)
{
	assert(isselecting);
	end = cursor.getPosition();
}

void TextSelection::ensureSelecting(EditboxCursor &cursor)
{
	if(!isselecting)
	{
		restartSelection(cursor);
	}
}

void EditboxCursor::operator ++(int)
{
	if((unsigned int)index < host.getBuffer().size()-1)
		index++;
	setPreferredX();
}

void EditboxCursor::operator --(int)
{
	if(index > 0)
		index--;
	setPreferredX();
}

void EditboxCursor::setPreferredX()
{
	preferredX = host.findCursor().x;
}

void EditboxCursor::insertChar(int c)
{
	if(host.isReadonly())
		return;
	Unicode::insertAtIndex(host.getBuffer(),c,index);
	CursorPos cp = host.findCursor();
	Unicode::insertAtIndex(cp.it->line, c, cp.index);
	cp.it->numchars++;
	host.markAsDirty(cp.it);
	host.getView()->update();
	(*this)++;
}

void EditboxCursor::deleteChar()
{
	if(host.isReadonly())
		return;
	//DO NOT let you delete the last char of the buffer
	if(index == int(host.getBuffer().size())-1)
	{
		return;
	}
	Unicode::removeRange(host.getBuffer(),index,index+1);
	//check if newline was deleted
	CursorPos cp = host.findCursor();
	int c = Unicode::getCharAtIndex(cp.it->line, cp.index);
	if(c == '\n')
	{
		cp.it->newlineterminated = false;
	}
	Unicode::removeRange(cp.it->line, cp.index, cp.index+1);
	cp.it->numchars--;
	host.markAsDirty(cp.it);
	host.getView()->update();
}

void EditboxCursor::insertString(string s)
{
	if(host.isReadonly())
		return;
	int length = Unicode::getLength(s);
	for(int i=0; i<length; i++)
	{
		insertChar(Unicode::getCharAtIndex(s,i));
	}
}

void EditboxCursor::insertNewline()
{
	if(host.isReadonly())
		return;
	int c = Unicode::getCharAtOffset(uconvert_ascii("\n",NULL),0);
	Unicode::insertAtIndex(host.getBuffer(),c,index);
	CursorPos cp = host.findCursor();
	//split off the next line
	int offset = Unicode::indexToOffset(cp.it->line, cp.index);
	string newline = cp.it->line.substr(offset, cp.it->line.size()-offset);
	LineData newld = {newline, cp.it->numchars-cp.index, cp.it->newlineterminated,true,NULL};
	cp.it->line = cp.it->line.substr(0,offset);
	Unicode::insertAtIndex(cp.it->line, c, cp.index);
	cp.it->numchars = cp.index+1;
	cp.it->newlineterminated = true;
	cp.it->dirtyflag = true;
	list<LineData>::iterator next = cp.it;
	next++;
	host.getLines().insert(next,newld);
	next--;
	host.markAsDirty(next);
	host.markAsDirty(cp.it);
	host.getView()->update();
	(*this)++;
}

CursorPos EditboxModel::findCursor()
{
	return findIndex(cursor.getPosition());
}

CursorPos EditboxModel::findIndex(int totalindex)
{
	int lineno = 0;
	int curindex = 0;
	CursorPos rval;
	for(list<LineData>::iterator it = lines.begin(); it != lines.end(); it++)
	{
		curindex += it->numchars;
		rval.it = it;
		if(curindex > totalindex)
			break;
		lineno++;
	}
	int offinline = totalindex-curindex+rval.it->numchars;
	rval.lineno = lineno;
	rval.index = offinline;
	string &str = rval.it->line;
	rval.x = 0;
	for(int i=0; i<offinline; i++)
	{
		int c = Unicode::getCharAtIndex(str,i);
		rval.x += Unicode::getCharWidth(c,view->getFont());
	}
	return rval;
}

void EditboxModel::markAsDirty(list<LineData>::iterator line)
{
	//coalesce lines that are NOT newline-terminated
	list<LineData>::reverse_iterator rit = list<LineData>::reverse_iterator(line);
	//find start of line
	while(rit != lines.rend() && !rit->newlineterminated)
	{
		rit++;
	}
	line = rit.base();
	while(!line->newlineterminated)
	{
		list<LineData>::iterator next = line;
		next++;
		assert(next != lines.end());
		line->numchars += next->numchars;
		destroy_bitmap(next->strip);
		line->line += next->line;
		line->newlineterminated = next->newlineterminated;
		lines.erase(next);
	}
	line->dirtyflag=true;
}

void EditboxModel::copy()
{
	if(!getSelection().hasSelection())
		return;
	pair<int,int> sel = getSelection().getSelection();
	Unicode::extractRange(getBuffer(), clipboard, sel.first, sel.second);
}

void EditboxModel::cut()
{
	copy();
	if(!isReadonly())
		clear();
}

void EditboxModel::clear()
{
	if(isReadonly())
		return;
	//ultimate annoying.
	//kill some lines
	if(!getSelection().hasSelection())
		return;
	pair<int, int> sel = getSelection().getSelection();
	getCursor().updateCursor(sel.first);
	getSelection().clearSelection();
	Unicode::removeRange(getBuffer(),sel.first, sel.second);
	CursorPos start = findIndex(sel.first);
	CursorPos end = findIndex(sel.second);
	if(start.lineno == end.lineno)
	{
		//thank god, an easy case
		//slight complication if the \n was deleted
		if(end.index == end.it->numchars)
			end.it->newlineterminated = false;
		int numremoved = end.index-start.index;
		Unicode::removeRange(end.it->line, start.index, end.index);
		end.it->numchars -= numremoved;
		markAsDirty(end.it);
		getView()->update();
		return;
	}
	else
	{
		//truncate starting line
		start.it->newlineterminated = false;
		start.it->line = start.it->line.substr(0, start.index);
		start.it->numchars = start.index;
		//count the number of lines to munch
		list<LineData>::iterator it = start.it;
		int numtomunch=0;
		for(it++; it != end.it; ++it) ++numtomunch;
		//and now munch
		for(int i=0; i<numtomunch; i++)
		{
			it = start.it;
			it++;
			destroy_bitmap(it->strip);
			getLines().erase(it);
		}
        //munch the last line
		it = start.it;
		it++;
		//there's a complication if the whole line is deleted
		if(it->numchars == end.index)
		{
			it->newlineterminated = false;
		}
		it->numchars -= end.index;
		it->line = it->line.substr(end.index, it->line.size()-end.index);
		markAsDirty(start.it);
		getView()->update();
	}
}

void EditboxModel::paste()
{
	if(isReadonly())
		return;
	CursorPos cp = findCursor();
	int offset = Unicode::indexToOffset(getBuffer(), getCursor().getPosition());
	buffer = buffer.substr(0,offset) + clipboard + buffer.substr(offset, buffer.size()-offset);
	//nevermind, THIS is the ultimate annoying
	//break up the lines int he clipboard
	list<LineData> toinsert;
	makeLines(toinsert,clipboard);
	//now split up the line being pasted onto
	offset = Unicode::indexToOffset(cp.it->line, cp.index);
	string therest = cp.it->line.substr(offset, cp.it->line.size()-offset);
	LineData therestline;
	therestline.line = therest;
	therestline.numchars = cp.it->numchars-cp.index;
	therestline.newlineterminated = cp.it->newlineterminated;
	therestline.strip = NULL;
	cp.it->line = cp.it->line.substr(0,offset);
	cp.it->newlineterminated = false;
	cp.it->numchars = cp.index;
	//insert the damn lines
	list<LineData>::iterator it = cp.it;
	it++;
	getLines().insert(it, therestline);
	for(list<LineData>::reverse_iterator toiit = toinsert.rbegin(); toiit != toinsert.rend(); toiit++)
	{
		list<LineData>::iterator iter = cp.it;
		iter++;
		getLines().insert(iter, *toiit);
	}
	it = cp.it;
	it++;
	for(unsigned int i=0; i<toinsert.size(); i++)
		it++;
	markAsDirty(it);
	//maybe mark starting line too
	if(toinsert.size() > 1 || (toinsert.size() == 1 && toinsert.begin()->newlineterminated == true))
	{
		markAsDirty(cp.it);
	}
	//update cursor
	getCursor().updateCursor(getCursor().getPosition()+Unicode::getLength(clipboard));
	getView()->update();
}

void EditboxModel::makeLines(list<LineData> &target, string &source)
{
  target.clear();
  string &str = source;
  const char *buf = str.c_str();
  int startoffset = 0;
  int startindex = 0;
  int endindex = 0;
  int lineno;
  lineno=0;
  int endoffset;
  for(endoffset = 0; endoffset != int(str.size()); endindex++)
  {
    int width = Unicode::getCharWidth(buf, endoffset);
    int c = Unicode::getCharAtOffset(buf, endoffset);
    if(c == '\n')
    {
      //end of line
      endoffset+=width;
      endindex++;
      LineData ld;
      ld.line = str.substr(startoffset, endoffset-startoffset);
      ld.numchars = endindex-startindex;
      ld.newlineterminated = true;
      ld.dirtyflag = true;
      ld.strip = NULL;
      target.push_back(ld);
      startindex = endindex+1;
      startoffset = endoffset;
      continue;
    }
    endoffset+=width;
  }
  if(endoffset != startoffset)
  {
	  LineData ld;
	  ld.line = str.substr(startoffset, endoffset-startoffset);
	  ld.numchars = endindex-startindex;
	  ld.newlineterminated = false;
	  ld.dirtyflag = true;
	  ld.strip = NULL;
	  target.push_back(ld);
  }
}

void EditboxModel::doHelp()
{
	string helpstr = "";
	if(!helpfile)
		return;
	FILE *hb = fopen(helpfile, "r");
  if(!hb)
  {
    return;
  }
  char c = fgetc(hb);
  while(!feof(hb))
  {
		helpstr+=c;
		c = fgetc(hb);
  }
  fclose(hb);

  help_dlg[0].dp2= lfont;
  if(is_large)
  {
    help_dlg[0].w=800;
    help_dlg[0].h=600;
    help_dlg[1].w=800-8;
    help_dlg[1].h=600-27;
    help_dlg[2].w=800-8-4;
    help_dlg[2].h=600-27-4;
  }
  help_dlg[2].dp = new EditboxModel(helpstr, new EditboxWordWrapView(&help_dlg[2],(is_large?sfont3:pfont),view->getDialog()->fg,view->getDialog()->bg,BasicEditboxView::HSTYLE_EOTEXT),true);
  help_dlg[2].bg = view->getDialog()->bg;
  zc_popup_dialog(help_dlg,2);
  delete (EditboxModel*)(help_dlg[2].dp);
}
 
