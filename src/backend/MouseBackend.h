#ifndef MOUSEBACKEND_H
#define MOUSEBACKEND_H

struct BITMAP;

class MouseBackend
{
public:
	~MouseBackend();

	/*
	* Sets the sprite to use for the mouse cursor. No cursor will be visible
	* until a sprite is set using this call (but the mouse otherwise still
	* function, i.e. the effective mouse position can be queried using
	* getVirtualScreenX() even if no cursor is currently visible there.)
	* The mouse cursor is drawn in virtual screen space, i.e., it will be scaled
	* when it is rendered to the framebuffer.
	* Note that this call does not automatically show the cursor; cursor
	* visibility is controlled by setCursorVisibility().
	*
	* IMPORTANT: the mouse backend does not currently make a deep copy of the 
	* bitmap passed in. Instead, the backend assumes that bmp will not change
	* size, and will not be deallocated, until the next call to setCursorSprite
	* (or until the mouse backend is destroyed).
	*/
	void setCursorSprite(BITMAP *bmp);

	/*
	* Sets whether the mouse cursor should be drawn onto the screen each frame.
	* This is cosmetic only: the mouse position can be still be queried even
	* while the cursor is invisible.
	* Note that even if the mouse cursor is set to be visible, nothing will
	* be drawn until the cursor bitmap has been set using setCursorSprite().
	*/
	void setCursorVisibility(bool visible);

	/*
	* Retrieves the current position, *in virtual screen space*, of the mouse
	* cursor tip.
	*/
	int getVirtualScreenX();
	int getVirtualScreenY();

	/*
	* Returns the current position of the mouse wheel, in relative "wheel
	* clicks" away from the wheel's position at program start.
	* A negative sign indicates a net downward wheel turn.
	*/
	int getWheelPosition();

	/*
	* Returns the state of the mouse buttons. left-, right-, and middle-
	* ButtonClicked() will return true if the corresponding button is
	* currently pressed. anyButtonClicked() returns true if any of the
	* three (in any combination) is pressed.
	*/
	bool anyButtonClicked();
	bool leftButtonClicked();
	bool rightButtonClicked();
	bool middleButtonClicked();

	/*
	* Sets the position of the mouse cursor tip, in virtual screen space.
	* The actual cursor sprite might be drawn offset to this position, 
	* depending on the value of the offset specified in setCursorTip().
	*/
	void setVirtualScreenPos(int x, int y);

	/*
	* Sets the current mouse wheel position. After this call, mouse wheel
	* rotation is measured starting from this value, e.g. if the user
	* rotates the mouse wheel down two clicks after setWheelPosition(6),
	* getWheelPosition() will return 4.
	*/
	void setWheelPosition(int pos);

	/*
	* Clamps mouse motion to stay within the rectangle specified by (x1,y1) and
	* (x2,y2), in virtual screen space. Assumes x1 <= x2 and y1 <= y2.
	* Overwrites the bounds set by any previous calls to this function.
	*/
	void setVirtualScreenMouseBounds(int x1, int y1, int x2, int y2);

	/*
	* Sets the offset of the cursor tip, with respect to the cursor bitmap's
	* top-left corner. For example, setCursorTip(10,10) indicated that the
	* cursor tip is located at pixel (10,10) of the cursor sprite, so that
	* the sprite will be drawn starting from getVirtualScreenX() - 10,
	* getVirtualScreenY()-10, etc.
	*/
	void setCursorTip(int x, int y);

	/*
	* Draws the mouse cursor onto screenbmp, using the appropriate
	* cursor sprite and cursor tip offset set using setCursorSprite()
	* and setCursorTip().
	* Does nothing if the mouse cursor is currently invisible (see
	* setCursorVisibility()).
	* Also a backup of the region of screenbmp overwritten by the cursor,
	* so that this region can be restored later using unrenderCursor().
	*/
	void renderCursor(BITMAP *screenbmp);

	/*
	* "Undoes" the previous call to renderCursor, restoring screenbmp with
	* whatever pixel data it contained before that call.
	* Does nothing if the sprite cursor is currently invisible.
	*
	* IMPORTANT: Currently this function is very simple, and assumes that
	* - renderCursor() has been called prior to this function;
	* - screenbmp has not been resized or destroyed since the last call to
	*   renderCursor();
	* - the region of screenbmp under the mouse cursor has not been modified
	*   since the last call to renderCursor();
	* - the mouse cursor sprite or tip position has not been modified since 
	*   the last call to renderCursor().
	* Calling unrenderCursor in circumstances violating these assumptions may
	* result in a program crash, and this function should only be used in
	* carefully controlled situations (namely, within the graphics backend).
	*/
	void unrenderCursor(BITMAP *screenbmp);
	
	friend class Backend;
private:
	MouseBackend();

	int focus_dx_, focus_dy_;

	BITMAP *cursor_;
	BITMAP *oldregion_;
	bool visible_;
	int oldx_, oldy_;
};


#endif
