#ifndef ZC_GUI_TILEANIM_FRAME_H
#define ZC_GUI_TILEANIM_FRAME_H

#include "widget.h"
#include "dialog_ref.h"

int tile_anim_proc(int msg,DIALOG *d,int c);

namespace GUI
{

class TileFrame: public Widget
{
public:
	TileFrame();

	void setTile(int value);
	void setCSet(int value);
	void setFrames(int value);
	void setSpeed(int value);
	void setDelay(int value);
	
	void resetAnim();
	
private:
	enum { tfr_tile, tfr_cset, tfr_frames, tfr_speed, tfr_delay, tfr_aclk, tfr_aframe, tfr_MAX };
	int data[tfr_MAX];
	DialogRef alDialog;
	
	void applyVisibility(bool visible) override;
	void realize(DialogRunner& runner) override;
	friend int ::tile_anim_proc(int,DIALOG *,int);
};

}

#endif
