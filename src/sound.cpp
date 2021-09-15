#include "sound.h"
#include "zc_sys.h"

// The number of active SoundLoops for each sound.
static int loopCount[WAV_COUNT];

static void startLoop(short id)
{
	++loopCount[id];
	if(loopCount[id] == 1)
		cont_sfx(id);
}

static void stopLoop(short id)
{
	--loopCount[id];
	if(loopCount[id] == 0)
		stop_sfx(id);
}



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



SoundLoop::SoundLoop(): SoundLoop(-1)
{}

SoundLoop::SoundLoop(short id):
	id(id > 0 && id < WAV_COUNT? id: -1),
	playing(false)
{}

SoundLoop::SoundLoop(const SoundLoop& other): id(other.id), playing(false)
{}

SoundLoop& SoundLoop::operator=(short newID)
{
	bool wasPlaying = playing;
	stop();

	if(newID > 0 && newID < WAV_COUNT)
	{
		id = newID;
		if(wasPlaying)
		start();
	}
	else
		id = -1;

	return *this;
}

SoundLoop& SoundLoop::operator=(const SoundLoop& other)
{
	// Continue playing if THIS sound loop was playing,
	// not if the other one was.
	bool wasPlaying = playing;
	stop();
	id = other.id;
	if(wasPlaying)
		start();

	return *this;
}

SoundLoop::~SoundLoop()
{
	if(id > 0 && playing)
		stopLoop(id);
}

void SoundLoop::start()
{
	// TODO: Actually, the sound should be marked as playing even if
	// the ID is invalid. Then if the sound is changed, it'll start
	// playing, as it should.
	if(id > 0 && !playing)
	{
		playing = true;
		startLoop(id);
	}
}

void SoundLoop::stop()
{
	if(playing)
	{
		playing = false;
		stopLoop(id);
	}
}
