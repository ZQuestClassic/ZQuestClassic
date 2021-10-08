#include "sound.h"
#include "zc_sys.h"

Sound::Sound(): Sound(-1)
{}

Sound::Sound(short id): id(id>0 && id<WAV_COUNT? id: -1)
{}

Sound::Sound(const Sound& other): id(other.id)
{}

Sound& Sound::operator=(short newID)
{
	if(newID > 0 && newID < WAV_COUNT)
		id = newID;
	else
		id = -1;
	return *this;
}

Sound& Sound::operator=(const Sound& other)
{
	id = other.id;
	return *this;
}

void Sound::play() const
{
	if(id > 0)
		sfx(id, 128, false);
}

void Sound::play(zfix x) const
{
	if(id > 0)
		sfx(id, pan((int)x), false);
}

void Sound::play(short id)
{
	if(id > 0 && id < WAV_COUNT)
		sfx(id, 128, false);
}

void Sound::play(short id, zfix x)
{
	if(id > 0 && id < WAV_COUNT)
		sfx(id, pan((int)x), false);
}
