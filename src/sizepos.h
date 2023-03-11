#ifndef _SIZEPOS_H_
#define _SIZEPOS_H_
#include "base/zdefs.h"

struct size_and_pos
{
	int x = -1, y = -1;
	int w = -1, h = -1;
	int xscale = 1, yscale = 1;
	int fw = -1, fh = -1;
	
	int data[8] = {0};
	
	//Get virtual values
	int tw() const;
	int th() const;
	
	virtual void clear(); //Clear to default vals
	
	bool rect(int mx, int my) const; //Check rect collision
	int rectind(int mx, int my) const; //Check scaled collision
	
	//Set coord values
	void set(int nx, int ny, int nw, int nh);
	void set(int nx, int ny, int nw, int nh, int xs, int ys);
	
	size_and_pos const& subsquare(int ind) const;
	size_and_pos const& subsquare(int col, int row) const;
	size_and_pos(int nx = -1, int ny = -1, int nw = -1, int nh = -1, int xsc = 1, int ysc = 1, int fw = -1, int fh = -1);
};

struct highlight_pos : public size_and_pos
{
	int thick = 2, color = 0xFF00FF;
	ALLEGRO_BITMAP* tbmp = nullptr;
	virtual void clear(); //Clear to default vals
	void clear_bmps();
	void highlight();
	
private:
	std::vector<ALLEGRO_BITMAP*> clearbmps;
};

void highlight_sqr(ALLEGRO_COLOR color, float x, float y, float w, float h, float thick = 2);
void highlight_sqr(ALLEGRO_COLOR color, size_and_pos const& rec, float thick = 2);
void highlight_frag(ALLEGRO_COLOR color, float x1, float y1, float w, float h, float fw, float fh, int thick = 2);
void highlight_frag(ALLEGRO_COLOR color, size_and_pos const& rec, int thick = 2);

#endif


