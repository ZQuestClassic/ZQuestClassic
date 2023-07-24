#include "msgstr.h"

MsgStr *MsgStrings;

// Copy everything except listpos
MsgStr& MsgStr::operator=(MsgStr const& other)
{
	copyText(other);
	copyStyle(other);
	return *this;
}

// Copy text data - just s and nextstring
void MsgStr::copyText(MsgStr const& other)
{
	s.resize(other.s.size());
	s.assign(other.s.c_str());
	nextstring=other.nextstring;
}

// Copy style data - everything except s, nextstring, and listpos
void MsgStr::copyStyle(MsgStr const& other)
{
	tile=other.tile;
	cset=other.cset;
	trans=other.trans;
	font=other.font;
	x=other.x;
	y=other.y;
	w=other.w;
	h=other.h;
	sfx=other.sfx;
	vspace=other.vspace;
	hspace=other.hspace;
	stringflags=other.stringflags;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = other.margins[q];
	}
	portrait_tile=other.portrait_tile;
	portrait_cset=other.portrait_cset;
	portrait_x=other.portrait_x;
	portrait_y=other.portrait_y;
	portrait_tw=other.portrait_tw;
	portrait_th=other.portrait_th;
	shadow_type=other.shadow_type;
	shadow_color=other.shadow_color;
	drawlayer=other.drawlayer;
}

void MsgStr::copyAll(MsgStr const& other)
{
	copyText(other);
	copyStyle(other);
	listpos = other.listpos;
}

void MsgStr::clear()
{
	s = "";
	s.shrink_to_fit();
	nextstring = 0;
	tile=0;
	cset=0;
	trans=0;
	font=0;
	x=0;
	y=0;
	w=0;
	h=0;
	sfx=0;
	vspace=0;
	hspace=0;
	stringflags=0;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = 0;
	}
	portrait_tile=0;
	portrait_cset=0;
	portrait_x=0;
	portrait_y=0;
	portrait_tw=0;
	portrait_th=0;
	shadow_type=0;
	shadow_color=0;
	listpos=0;
	drawlayer=6;
}

bool valid_str(char const* ptr, char cancel)
{
	return ptr && ptr[0] && ptr[0] != cancel;
}

