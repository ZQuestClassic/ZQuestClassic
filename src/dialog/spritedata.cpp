#include "spritedata.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "zc_list_data.h"

void mark_save_dirty();

void call_sprite_dlg(int32_t index)
{
	if(unsigned(index) >= MAXSPRITES) return;
	SpriteDataDialog(index).show();
}

SpriteDataDialog::SpriteDataDialog(int32_t index):
	index(index), tempSprite(sprite_data_buf.get(index))
{
	if (index >= sprite_data_buf.capacity() && index < MAXSPRITES)
		tempSprite.name = fmt::format("zz{:03}", index);
}

#define NUM_FIELD(member,_min,_max) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = tempSprite.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		tempSprite.member = val; \
	})

#define ANIM_NUM_FIELD(member,_min,_max) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = tempSprite.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		tempSprite.member = val; \
		updateAnimation(); \
	})

void SpriteDataDialog::updateAnimation()
{
	auto flip = tempSprite.flip();
	auto cs = tempSprite.csets&0xF;
	tswatch->setCSet(cs);
	tswatch->setFlip(flip);
	animFrame->setCSet(cs);
	animFrame->setFlip(flip);
	animFrame->setFrames(tempSprite.frames);
	animFrame->setSpeed(tempSprite.speed);
	animFrame->setTile(tempSprite.tile);
}

std::shared_ptr<GUI::Widget> SpriteDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window = Window(
		use_vsync = true,
		title = fmt::format("Sprite {}: {}", index, tempSprite.name),
		minwidth = 30_em,
		info = "Sprite Data is used as misc animation data for other objects.",
		onClose = message::CANCEL,
		Column(
			Row(
				Label(text = "Name:", hAlign = 0.0),
				TextField(
					maxwidth = 15_em,
					maxLength = 64,
					text = tempSprite.name,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
					{
						tempSprite.name = str;
						window->setTitle(fmt::format("Sprite {}: {}", index, tempSprite.name));
					}
				)
			),
			Rows<5>(
				DINFOBTN(),
				Label(text = "Flash CSet", hAlign = 0.0),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
					low = 0, high = 15, val = (tempSprite.csets>>4),
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						tempSprite.csets &= 0x0F;
						tempSprite.csets |= ((val&0xF)<<4);
					}),
				//
				Column(padding = 0_px, rowSpan = 5,
					tswatch = SelTileSwatch(
						tile = tempSprite.tile,
						cset = tempSprite.csets&0x0F,
						flip = tempSprite.flip(),
						showFlip = true,
						showvals = false,
						onSelectFunc = [&](int32_t t, int32_t c, int32_t f,int32_t)
						{
							tempSprite.tile = t;
							tempSprite.csets &= 0xF0;
							tempSprite.csets |= (c&0xF);
							SETFLAG(tempSprite.misc,WF_HFLIP,f&1);
							hflipcb->setChecked(f&1);
							SETFLAG(tempSprite.misc,WF_VFLIP,f&2);
							vflipcb->setChecked(f&2);
							updateAnimation();
						}
					),
					Checkbox(
						text = "Auto-flash",
						hAlign = 0.0, fitParent = true,
						checked = tempSprite.misc & WF_AUTOFLASH,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(tempSprite.misc,WF_AUTOFLASH,state);
						}
					),
					Checkbox(
						text = "2P flash",
						hAlign = 0.0, fitParent = true,
						checked = tempSprite.misc & WF_2PFLASH,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(tempSprite.misc,WF_2PFLASH,state);
						}
					),
					hflipcb = Checkbox(
						text = "H-Flip",
						hAlign = 0.0, fitParent = true,
						checked = tempSprite.misc & WF_HFLIP,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(tempSprite.misc,WF_HFLIP,state);
							updateAnimation();
						}
					),
					vflipcb = Checkbox(
						text = "V-Flip",
						hAlign = 0.0, fitParent = true,
						checked = tempSprite.misc & WF_VFLIP,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(tempSprite.misc,WF_VFLIP,state);
							updateAnimation();
						}
					),
					Checkbox(
						text = "Behind",
						hAlign = 0.0, fitParent = true,
						checked = tempSprite.misc & WF_BEHIND,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(tempSprite.misc,WF_BEHIND,state);
						}
					)
				),
				Column(padding = 0_px, rowSpan = 5,
					animFrame = TileFrame(
						tile = tempSprite.tile,
						cset = tempSprite.csets&0xF,
						frames = tempSprite.frames,
						speed = tempSprite.speed,
						flip = tempSprite.flip()
					)
				),
				//
				INFOBTN("The number of frames in the animation"),
				Label(text = "Animation Frames", hAlign = 0.0),
				ANIM_NUM_FIELD(frames,0,255),
				//
				INFOBTN("The time to spend on each frame in the animation"),
				Label(text = "Animation Speed", hAlign = 0.0),
				ANIM_NUM_FIELD(speed,0,255),
				//
				INFOBTN("Used by almost nothing in engine. Probably just sword beams?"),
				Label(text = "Misc Type", hAlign = 0.0),
				NUM_FIELD(type,0,255)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool SpriteDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			sprite_data_buf[index] = tempSprite;
			mark_save_dirty();
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}
