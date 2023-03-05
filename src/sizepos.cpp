#include "sizepos.h"
#include "jwin_a5.h"

int size_and_pos::tw() const
{
	return w*xscale;
}
int size_and_pos::th() const
{
	return h*yscale;
}

int size_and_pos::cx() const
{
	return x+tw()/2;
}
int size_and_pos::cy() const
{
	return y+th()/2;
}

void size_and_pos::clear()
{
	x = -1, y = -1;
	w = -1, h = -1;
	xscale = 1, yscale = 1;
	fw = -1, fh = -1;
	
	memset(data,0,sizeof(data));
}
bool size_and_pos::rect(int mx, int my) const
{
	if(x < 0 || y < 0 || w < 0 || h < 0)
		return false;
	auto sw = w * xscale;
	auto sh = h * yscale;
	if(fw > -1 && fh > -1)
		if(mx >= x+fw && my >= y+fh)
			return false;
	return isinRect(mx,my,x,y,x+sw-1,y+sh-1);
}
int size_and_pos::rectind(int mx, int my) const
{
	if(!rect(mx,my)) return -1; //not in rect
	//Where in rect?
	mx -= x;
	my -= y;
	auto row = (my / yscale);
	auto col = (mx / xscale);
	int ind = col + (row * w);
	return ind;
}
void size_and_pos::set(int nx, int ny, int nw, int nh)
{
	x = nx; y = ny;
	w = nw; h = nh;
}
void size_and_pos::set(int nx, int ny, int nw, int nh, int xs, int ys)
{
	x = nx; y = ny;
	w = nw; h = nh;
	xscale = xs; yscale = ys;
}
static size_and_pos nilsqr;
static size_and_pos tempsqr;
size_and_pos const& size_and_pos::subsquare(int ind) const
{
	if(w < 1 || h < 1)
		return nilsqr;
	return subsquare(ind%w, ind/w);
}
size_and_pos const& size_and_pos::subsquare(int col, int row) const
{
	if(w < 1 || h < 1)
		return nilsqr;
	int x2 = x+(col*xscale);
	int y2 = y+(row*yscale);
	if(fw > -1 && fh > -1 && x2 >= x+fw && y2 >= y+fh)
		return nilsqr;
	tempsqr.clear();
	tempsqr.set(x2,y2,xscale,yscale);
	return tempsqr;
}
size_and_pos::size_and_pos(int nx, int ny, int nw, int nh, int xsc, int ysc, int fw, int fh)
	: x(nx), y(ny), w(nw), h(nh), xscale(xsc), yscale(ysc), fw(fw), fh(fh)
{}

void highlight_pos::clear()
{
	size_and_pos::clear();
	thick = 2, color = 0xFF00FF;
	if(tbmp)
		clearbmps.push_back(tbmp);
	tbmp = nullptr;
}

void highlight_pos::clear_bmps()
{
	for(ALLEGRO_BITMAP* b : clearbmps)
		clear_a5_bmp(AL5_INVIS,b);
	clearbmps.clear();
}

void highlight_sqr(ALLEGRO_COLOR color, float x, float y, float w, float h, float thick)
{
	for(int q = 0; q < thick; ++q)
		al_draw_rectangle(x+q, y+q, x+w-q, y+h-q, color, 1);
}
void highlight_sqr(ALLEGRO_COLOR color, size_and_pos const& rec, float thick)
{
	highlight_sqr(color, rec.x, rec.y, rec.tw(), rec.th(), thick);
}
void highlight_frag(ALLEGRO_COLOR color, float x1, float y1, float w, float h, float fw, float fh, int thick)
{
	if(thick < 1) return;
	
	int xc = x1+fw-1;
	int yc = y1+fh-1;
	int x2 = x1+w-1;
	int y2 = y1+h-1;
	
	for(;thick > 0;--thick)
	{
		al_draw_hline(x1-1, y1-1, x2, color);
		al_draw_vline(x1, y1-1, y2, color);
		
		al_draw_hline(x1-1, y2, xc, color);
		al_draw_vline(x2+1, y1-1, yc+1, color);
		al_draw_hline(xc, yc, x2+1, color);
		al_draw_vline(xc+1, yc, y2+1, color);
		++x1; ++y1;
		--x2; --y2;
		--xc; --yc;
	}
}
void highlight_frag(ALLEGRO_COLOR color, size_and_pos const& rec, int thick)
{
	highlight_frag(color, rec.x, rec.y, rec.tw(), rec.th(), rec.fw, rec.fh, thick);
}

void highlight_pos::highlight()
{
	if(x<0||y<0) return;
	
	ALLEGRO_STATE old_state;
	
	if(tbmp)
	{
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
		al_set_target_bitmap(tbmp);
		clear_a5_bmp(AL5_INVIS);
	}
	
	clear_bmps();
	
	if(fw > -1 && fh > -1)
		highlight_frag(hexcolor(color), *this, thick);
	else highlight_sqr(hexcolor(color), *this, thick);
	
	if(tbmp)
		al_restore_state(&old_state);
}


