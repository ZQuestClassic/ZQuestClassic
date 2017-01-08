#include "MouseBackend.h"
#include "GraphicsBackend.h"
#include "Backend.h"
#include "../zc_alleg.h"
#include <cassert>

// For Allegro GUI routines

int ZC_gui_mouse_x()
{
	return Backend::mouse->getVirtualScreenX();
}

int ZC_gui_mouse_y()
{
	return Backend::mouse->getVirtualScreenY();
}

MouseBackend::MouseBackend() : visible_(false), cursor_(NULL), oldregion_(NULL), focus_dx_(0), focus_dy_(0), oldx_(0), oldy_(0)
{
	gui_mouse_x = &ZC_gui_mouse_x;
	gui_mouse_y = &ZC_gui_mouse_y;
	gui_mouse_focus = false;
}

MouseBackend::~MouseBackend()
{
	if (oldregion_)
		destroy_bitmap(oldregion_);
}

int MouseBackend::getVirtualScreenX()
{
	int x = mouse_x;
	int y = mouse_y;
	Backend::graphics->physicalToVirtual(x, y);
	return x;
}

int MouseBackend::getVirtualScreenY()
{
	int x = mouse_x;
	int y = mouse_y;
	Backend::graphics->physicalToVirtual(x, y);
	return y;
}

int MouseBackend::getWheelPosition()
{
	return mouse_z;
}

void MouseBackend::setCursorVisibility(bool visible)
{
	visible_ = visible;
}

void MouseBackend::setCursorSprite(BITMAP *bmp)
{
	cursor_ = bmp;
	if (bmp)
	{
		bool ok = oldregion_ && oldregion_->w >= bmp->w  && oldregion_->h >= bmp->h;
		if (!ok)
		{
			if(oldregion_)
				destroy_bitmap(oldregion_);
			oldregion_ = create_bitmap_ex(8, bmp->w, bmp->h);
		}
	}
}

bool MouseBackend::anyButtonClicked()
{
	return leftButtonClicked() || rightButtonClicked() || middleButtonClicked();
}

bool MouseBackend::leftButtonClicked()
{
	return (mouse_b & 1) != 0;
}

bool MouseBackend::rightButtonClicked()
{
	return (mouse_b & 2) != 0;
}

bool MouseBackend::middleButtonClicked()
{
	return (mouse_b & 4) != 0;
}

void MouseBackend::setVirtualScreenPos(int x, int y)
{
	Backend::graphics->virtualToPhysical(x, y);
	position_mouse(x, y);
}

void MouseBackend::setWheelPosition(int pos)
{
	position_mouse_z(pos);
}

void MouseBackend::setVirtualScreenMouseBounds(int x1, int y1, int x2, int y2)
{
	Backend::graphics->virtualToPhysical(x1, y1);
	Backend::graphics->virtualToPhysical(x2, y2);
	set_mouse_range(x1, y1, x2, y2);
}

void MouseBackend::setCursorTip(int x, int y)
{
	focus_dx_ = x;
	focus_dy_ = y;
}

void MouseBackend::renderCursor(BITMAP *screenbmp)
{
	if (!visible_ || !cursor_)
		return;

	oldx_ = getVirtualScreenX() - focus_dx_;
	oldy_ = getVirtualScreenY() - focus_dy_;	

	blit(screenbmp, oldregion_, oldx_, oldy_, 0, 0, cursor_->w, cursor_->h);
	masked_blit(cursor_, screenbmp, 0, 0, oldx_, oldy_, cursor_->w, cursor_->h);
}

void MouseBackend::unrenderCursor(BITMAP *screenbmp)
{
	if (!visible_ || !cursor_)
		return;

	blit(oldregion_ , screenbmp, 0, 0, oldx_, oldy_, cursor_->w, cursor_->h);
}
