#ifndef GRPAHICSBACKEND_H
#define GRAPHICSBACKEND_H

#include <vector>

struct BITMAP;

class GraphicsBackend
{
public:
	GraphicsBackend();
	~GraphicsBackend();
	
	void readConfigurationOptions(const std::string &prefix);
	void writeConfigurationOptions(const std::string &prefix);
	void setFullscreen(bool fullscreen);
	bool isFullscreen() { return fullscreen_; }

	void setScreenResolution(int width, int height);

	int screenW();
	int screenH();	

	bool initialize();
	
	int virtualScreenW() { return virtualw_; }
	int virtualScreenH() { return virtualh_; }

	void registerVirtualModes(const std::vector<std::pair<int, int> > &desiredModes);
	int getVirtualMode() { return curmode_; }

	int miniscreenX() { return (virtualScreenW() - 320) / 2; }
	int miniscreenY() { return (virtualScreenH() - 240) / 2; }

	void waitTick();
	void showBackBuffer();
	int getLastFPS();
	void setVideoModeSwitchDelay(int msec);

	void physicalToVirtual(int &x, int &y);
	void virtualToPhysical(int &x, int &y);	

	void onSwitchIn();
	
	friend void update_frame_counter();

private:
	bool trySettingVideoMode();

	BITMAP *hw_screen_;
	BITMAP *backbuffer_;

	bool initialized_;
	int screenw_, screenh_;
	bool fullscreen_;	
	
	std::vector<std::pair<int, int> > virtualmodes_;
	int curmode_;
	int virtualw_, virtualh_;

	int switchdelay_;
	int fps_;

	static volatile int frame_counter;
	static volatile int frames_this_second;
	static volatile int prev_frames_this_second;
};

#endif