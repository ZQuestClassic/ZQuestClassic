#ifndef PARTICLES_H_
#define PARTICLES_H_

#include "base/zdefs.h"
#include "base/zfix.h"
#include <map>

using std::map;

class particle
{
private:
    static int32_t getNextUID();
    int32_t uid;
    
public:
    int32_t getUID()
    {
        return uid;
    }
	zfix x, y, yofs;
	int32_t layer, cset, color;
	zfix step;
	int32_t dir;
	bool angular;
	double angle;
	particle(zfix X,zfix Y,int32_t layer,int32_t cset,int32_t color);
	virtual ~particle();
	virtual bool animate(int32_t index);
	virtual void draw(BITMAP *dest);
	virtual void move(zfix s);
};

class pDivineEscapeDust : public particle
{
public:
	zfix os;
	int32_t ot;
	int32_t timer;
	bool initialized;
	pDivineEscapeDust(zfix X,zfix Y,int32_t layer,int32_t cset,int32_t color,int32_t timer);
	virtual bool animate(int32_t index);
};

class pTwilight : public particle
{
public:
	int32_t delay;
	pTwilight(zfix X,zfix Y,int32_t layer,int32_t cset,int32_t color, int32_t delay);
	virtual bool animate(int32_t index);
};

#define PARTLIST_MAX (65535)

class particle_list
{
	particle *particles[PARTLIST_MAX];
	int32_t count;
	int32_t active_iterator;
	int32_t max_particles;
	map<int32_t, int32_t> containedUIDs;
	// Cache requests from scripts
	mutable int32_t lastUIDRequested;
	mutable particle* lastRequested;
public:
	particle_list();
	// Note: these are NOT sprite uids. Separate system. Particles are not sprites.
	particle *getByUID(int32_t uid);
	void clear();
	particle *at(int32_t index);
	bool swap(int32_t a, int32_t b);
	bool add(particle *p);
	bool remove(particle *p);
	zfix getX(int32_t j);
	zfix getY(int32_t j);
	int32_t getID(int32_t j);
	int32_t getMax() {return max_particles;}
	void setMax(int32_t max) {max_particles = (max < PARTLIST_MAX ? max : PARTLIST_MAX);}
	bool del(int32_t j);
	void draw(BITMAP* dest,bool lowfirst,int32_t lyr=-999);
	void animate();
	bool has_space(int32_t space);
	int32_t Count();
};

extern particle_list particles;

#endif
