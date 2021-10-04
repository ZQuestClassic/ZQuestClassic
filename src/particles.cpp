/**********************************/
/********  Particle Class  ********/
/**********************************/

#include "precompiled.h" //always first

#include "particles.h"

particle::~particle()
{
}

bool particle::animate(int)
{
	return true;
}

void particle::draw(BITMAP *dest)
{
	int tcs=cset;
	tcs &= 15;
	tcs <<= CSET_SHFT;
	putpixel(dest, x, y+yofs, tcs+color);
}

particle::particle(zfix X,zfix Y,int L,int CS,int C)
{
    uid = getNextUID();
	x=X;
	y=Y;
	layer=L;
	cset=CS;
	color=C;
	yofs = 54;
	dir = 0;
	step = 0;
}

void particle::move(zfix s)
{
    if(angular)
    {
        x += cos(angle)*s;
        y += sin(angle)*s;
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

long particle::getNextUID()
{
    static long nextid = 0;
    return nextid++;
}

pFaroresWindDust::pFaroresWindDust(zfix X,zfix Y,int L,int CS,int C, int T) : particle(X,Y,L,CS,C)
{
	initialized=false;
	timer=T;
}

bool pFaroresWindDust::animate(int)
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

pTwilight::pTwilight(zfix X,zfix Y,int L,int CS,int C, int T) : particle(X,Y,L,CS,C)
{
	dir = up;
	delay = T;
}

bool pTwilight::animate(int)
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

particle *particle_list::at(int index)
{
	if(index<0 || index>=count)
		return NULL;
		
	return particles[index];
}

bool particle_list::swap(int a,int b)
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
	//checkConsistency();
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
	
	map<long, int>::iterator it = containedUIDs.find(p->getUID());
	
	if(it != containedUIDs.end())
		containedUIDs.erase(it);
		
	int j=0;
	
	for(; j<count; j++)
		if(particles[j] == p)
			goto gotit;
			
	return false;
	
gotit:

	for(int i=j; i<count-1; i++)
	{
		particles[i]=particles[i+1];
		containedUIDs[particles[i]->getUID()] = i;
	}
	
	--count;
	//checkConsistency();
	return true;
}

zfix particle_list::getX(int j)
{
	if((j>=count)||(j<0))
	{
		return (zfix)1000000;
	}
	
	return particles[j]->x;
}

zfix particle_list::getY(int j)
{
	if((j>=count)||(j<0))
	{
		return (zfix)1000000;
	}
	
	return particles[j]->y;
}

bool particle_list::del(int j)
{
	if(j<0||j>=count)
		return false;
		
	map<long, int>::iterator it = containedUIDs.find(particles[j]->getUID());
	
	if(it != containedUIDs.end())
		containedUIDs.erase(it);
	
	if(particles[j]==lastRequested)
	{
		lastUIDRequested=0;
		lastRequested=0;
	}
	
	delete particles[j];
	
	for(int i=j; i<count-1; i++)
	{
		particles[i]=particles[i+1];
		containedUIDs[particles[i]->getUID()] = i;
	}
	
	--count;
	if(j<=active_iterator) --active_iterator;
	//checkConsistency();
	return true;
}

void particle_list::draw(BITMAP* dest,bool lowfirst,int lyr)
{
	if(lowfirst)
	{
		for(int i=0; i<count; i++)
		{
			if(lyr>-999 && particles[i]->layer != lyr)
				continue;
			particles[i]->draw(dest);
		}
	}
	else
	{
		for(int i=count-1; i>=0; i--)
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

int particle_list::Count()
{
	return count;
}

bool particle_list::has_space(int space)
{
	return (count+space) <= max_particles;
}

particle * particle_list::getByUID(long uid)
{
	if(uid==lastUIDRequested)
		return lastRequested;
	
	map<long, int>::iterator it = containedUIDs.find(uid);
	
	if(it != containedUIDs.end())
	{
		// Only update cache if requested particle was found
		lastUIDRequested=uid;
		lastRequested=at(it->second);
		return lastRequested;
	}
		
	return NULL;
}


/*** end of particle.cc ***/

