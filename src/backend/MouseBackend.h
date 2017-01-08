#ifndef MOUSEBACKEND_H
#define MOUSEBACKEND_H

struct BITMAP;

class MouseBackend
{
public:
	~MouseBackend();

	void setCursorVisibility(bool visible);

	void setCursorSprite(BITMAP *bmp);

	int getVirtualScreenX();
	int getVirtualScreenY();
	int getWheelPosition();

	bool anyButtonClicked();
	bool leftButtonClicked();
	bool rightButtonClicked();
	bool middleButtonClicked();

	void setVirtualScreenPos(int x, int y);
	void setWheelPosition(int pos);

	void setVirtualScreenMouseBounds(int x1, int y1, int x2, int y2);
	void setCursorTip(int x, int y);
	void renderCursor(BITMAP *screenbmp);
	void unrenderCursor(BITMAP *screenbmp);
	
	friend class GraphicsBackend;
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
