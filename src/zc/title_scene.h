#ifndef ZC_TITLE_SCENE_H_
#define ZC_TITLE_SCENE_H_

#include "core/title_theme.h"
#include <memory>

struct BITMAP;

// A holiday backdrop that the title screen draws in place of its plain blue.
class TitleScene
{
public:
	virtual ~TitleScene() = default;
	// Advances the scene a frame, and sets its colors in the palette.
	virtual void update() = 0;
	// Draws the whole scene over `dest`.
	virtual void draw(BITMAP* dest) = 0;
	// While the logo's intro plays, a scene holds back its showier effects so as not to
	// upstage it.
	void set_intro_playing(bool playing) { intro_playing = playing; }

protected:
	bool intro_playing = false;
};

bool is_halloween_today();
std::unique_ptr<TitleScene> create_halloween_title_scene(int w, int h);

#endif
