#ifndef _PARTICLES_H_
#define _PARTICLES_H_

#include "zdefs.h"
#include "zfix.h"
#include <map>

using std::map;

class particle
{
private:
    static long getNextUID();
    long uid;
    
public:
    long getUID()
    {
        return uid;
    }
	zfix x, y, yofs;
	int layer, cset, color;
	zfix step;
	int dir;
	bool angular;
	double angle;
	particle(zfix X,zfix Y,int layer,int cset,int color);
	virtual ~particle();
	virtual bool animate(int index);
	virtual void draw(BITMAP *dest);
	virtual void move(zfix s);
};

class pFaroresWindDust : public particle
{
public:
	zfix os;
	int ot;
	int timer;
	bool initialized;
	pFaroresWindDust(zfix X,zfix Y,int layer,int cset,int color,int timer);
	virtual bool animate(int index);
};

class pTwilight : public particle
{
public:
	int delay;
	pTwilight(zfix X,zfix Y,int layer,int cset,int color, int delay);
	virtual bool animate(int index);
};

#define PARTLIST_MAX (65535)

class particle_list
{
	particle *particles[PARTLIST_MAX];
	int count;
	int active_iterator;
	long max_particles;
	map<long, int> containedUIDs;
	// Cache requests from scripts
	mutable long lastUIDRequested;
	mutable particle* lastRequested;
public:
	particle_list();
	particle *getByUID(long uid);
	void clear();
	particle *at(int index);
	bool swap(int a, int b);
	bool add(particle *p);
	bool remove(particle *p);
	zfix getX(int j);
	zfix getY(int j);
	int getID(int j);
	long getMax() {return max_particles;}
	void setMax(long max) {max_particles = (max < PARTLIST_MAX ? max : PARTLIST_MAX);}
	bool del(int j);
	void draw(BITMAP* dest,bool lowfirst,int lyr=-999);
	void animate();
	bool has_space(int space);
	int Count();
};


#endif
/*** end of sprite.cc ***/

