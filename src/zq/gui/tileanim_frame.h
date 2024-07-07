#ifndef ZC_GUI_TILEANIM_FRAME_H_
#define ZC_GUI_TILEANIM_FRAME_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

int tile_anim_proc(int32_t msg,DIALOG *d,int32_t c);

namespace GUI
{

class TileFrame: public Widget
{
public:
	TileFrame();

	void setTile(int32_t value);
	void setCSet(int32_t value);
	void setCSet2(int32_t value);
	void setFrames(int32_t value);
	void setSpeed(int32_t value);
	void setDelay(int32_t value);
	void setSkipX(int32_t value);
	void setSkipY(int32_t value);
	void setFlip(int32_t value);
	void setDoSized(bool value);
	
	void resetAnim();
	
private:
	enum { tfr_tile, tfr_cset, tfr_frames, tfr_speed, tfr_delay, tfr_aclk, tfr_aframe, tfr_skipx, tfr_skipy, tfr_flip, tfr_cset2, tfr_dosized, tfr_MAX };
	int32_t data[tfr_MAX];
	DialogRef alDialog;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	friend int ::tile_anim_proc(int32_t,DIALOG *,int32_t);
};

}

#endif
