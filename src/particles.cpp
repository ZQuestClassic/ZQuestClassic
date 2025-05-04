#include "particles.h"
#include "base/zc_math.h"

particle::~particle()
{
}

bool particle::animate(int32_t)
{
	return true;
}

void particle::draw(BITMAP *dest)
{
	int32_t tcs=cset;
	tcs &= 15;
	tcs <<= CSET_SHFT;
	putpixel(dest, x, y+yofs, tcs+color);
}

particle::particle(zfix X,zfix Y,int32_t L,int32_t CS,int32_t C)
{
    uid = 0;
	x=X;
	y=Y;
	layer=L;
	cset=CS;
	color=C;
	yofs = 54;
	dir = 0;
	step = 0;
	angular = false;
	angle = 0;
}

void particle::move(zfix s)
{
    if(angular)
    {
        x += zc::math::Cos(angle)*s;
        y += zc::math::Sin(angle)*s;
		return;
    }
	
	if(dir<0) return;
	switch(NORMAL_DIR(dir))
	{
		case up:
			y-=s;
			break;
			
		case down:
			y+=s;
			break;
			
		case left:
			x-=s;
			break;
			
		case right:
			x+=s;
			break;
			
		case l_up:
			x-=s;
			y-=s;
			break;
			
		case r_up:
			x+=s;
			y-=s;
			break;
			
		case l_down:
			x-=s;
			y+=s;
			break;
			
		case r_down:
			x+=s;
			y+=s;
			break;
	}
}

int32_t particle::getNextUID()
{
    static int32_t nextid = 0;
    return nextid++;
}

pDivineEscapeDust::pDivineEscapeDust(zfix X,zfix Y,int32_t L,int32_t CS,int32_t C, int32_t T) : particle(X,Y,L,CS,C)
{
	initialized=false;
	timer=T;
}

bool pDivineEscapeDust::animate(int32_t)
{
	if(!initialized)
	{
		os=step;
		ot=timer;
		initialized=true;
	}
	
	step=os*(double)timer/(double)ot;
	
	if(timer>0)
	{
		--timer;
	}
	
	move(step);
	return (!timer);
}

pTwilight::pTwilight(zfix X,zfix Y,int32_t L,int32_t CS,int32_t C, int32_t T) : particle(X,Y,L,CS,C)
{
	dir = up;
	delay = T;
}

bool pTwilight::animate(int32_t)
{
	if(delay>0)
	{
		delay--;
	}
	else
	{
		move(step);
	}
	
	return (y<0)!=0;
}

// Particle List
particle_list::particle_list() : count(0), active_iterator(0), max_particles(255) {}
void particle_list::clear()
{
	while(count>0) del(0);
	lastUIDRequested=0;
	lastRequested=0;
}

particle *particle_list::at(int32_t index)
{
	if(index<0 || index>=count)
		return NULL;
		
	return particles[index];
}

bool particle_list::swap(int32_t a,int32_t b)
{
	if(a<0 || a>=count || b<0 || b>=count)
		return false;
		
	particle *c = particles[a];
	particles[a] = particles[b];
	particles[b] = c;
	containedUIDs[particles[a]->getUID()] = a;
	containedUIDs[particles[b]->getUID()] = b;
	return true;
}

bool particle_list::add(particle *p)
{
	if(count>=max_particles)
	{
		delete p;
		return false;
	}
	
	containedUIDs[p->getUID()] = count;
	particles[count++]=p;
	return true;
}

bool particle_list::remove(particle *p)
// removes pointer from list but doesn't delete it
{
	if(p==lastRequested)
	{
		lastUIDRequested=0;
		lastRequested=0;
	}
	
	map<int32_t, int32_t>::iterator it = containedUIDs.find(p->getUID());
	
	if(it != containedUIDs.end())
		containedUIDs.erase(it);
		
	int32_t j=0;
	
	for(; j<count; j++)
		if(particles[j] == p)
			goto gotit;
			
	return false;
	
gotit:

	for(int32_t i=j; i<count-1; i++)
	{
		particles[i]=particles[i+1];
		containedUIDs[particles[i]->getUID()] = i;
	}
	
	--count;
	return true;
}

zfix particle_list::getX(int32_t j)
{
	if((j>=count)||(j<0))
	{
		return (zfix)1000000;
	}
	
	return particles[j]->x;
}

zfix particle_list::getY(int32_t j)
{
	if((j>=count)||(j<0))
	{
		return (zfix)1000000;
	}
	
	return particles[j]->y;
}

bool particle_list::del(int32_t j)
{
	if(j<0||j>=count)
		return false;
		
	map<int32_t, int32_t>::iterator it = containedUIDs.find(particles[j]->getUID());
	
	if(it != containedUIDs.end())
		containedUIDs.erase(it);
	
	if(particles[j]==lastRequested)
	{
		lastUIDRequested=0;
		lastRequested=0;
	}
	
	delete particles[j];
	
	for(int32_t i=j; i<count-1; i++)
	{
		particles[i]=particles[i+1];
		containedUIDs[particles[i]->getUID()] = i;
	}
	
	--count;
	if(j<=active_iterator) --active_iterator;
	return true;
}

void particle_list::draw(BITMAP* dest,bool lowfirst,int32_t lyr)
{
	if(lowfirst)
	{
		for(int32_t i=0; i<count; i++)
		{
			if(lyr>-999 && particles[i]->layer != lyr)
				continue;
			particles[i]->draw(dest);
		}
	}
	else
	{
		for(int32_t i=count-1; i>=0; i--)
		{
			if(lyr>-999 && particles[i]->layer != lyr)
				continue;
			particles[i]->draw(dest);
		}
	}
}

void particle_list::animate()
{
	active_iterator = 0;
	
	while(active_iterator<count)
	{
		if(particles[active_iterator]->animate(active_iterator))
		{
			del(active_iterator);
		}
		
		++active_iterator;
	}
	active_iterator = -1;
}

int32_t particle_list::Count()
{
	return count;
}

bool particle_list::has_space(int32_t space)
{
	return (count+space) <= max_particles;
}

particle * particle_list::getByUID(int32_t uid)
{
	if(uid==lastUIDRequested)
		return lastRequested;
	
	map<int32_t, int32_t>::iterator it = containedUIDs.find(uid);
	
	if(it != containedUIDs.end())
	{
		// Only update cache if requested particle was found
		lastUIDRequested=uid;
		lastRequested=at(it->second);
		return lastRequested;
	}
		
	return NULL;
}

particle_list particles;
