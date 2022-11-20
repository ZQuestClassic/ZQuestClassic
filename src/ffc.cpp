#include "ffc.h"
#include "base/zdefs.h"

ffcdata::ffcdata() : solid_object()
{
	clear();
}

ffcdata::~ffcdata()
{}

ffcdata::ffcdata(ffcdata const& other)
	: solid_object(other), ax(other.ax), ay(other.ay), flags(other.flags),
	data(other.data), delay(other.delay), cset(other.cset), link(other.link),
	txsz(other.txsz), tysz(other.tysz), initialized(other.initialized)
{
	memcpy(initd, other.initd, sizeof(initd));
	memcpy(inita, other.inita, sizeof(inita));
}

void ffcdata::clear()
{
	x = y = vx = vy = ax = ay = 0;
	flags = 0;
	data = delay = 0;
	cset = link = 0;
	txsz = tysz = 1;
	hxsz = hysz = 16;
	initialized = false;
	memset(initd, 0, sizeof(initd));
	memset(inita, 0, sizeof(inita));
	setSolid(false);
}

void ffcdata::setSolid(bool set) //exists so that ffcs can do special handling for whether to make something solid or not.
{
	solid_object::setSolid(set);
}

