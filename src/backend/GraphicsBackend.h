#ifndef GRPAHICSBACKEND_H
#define GRAPHICSBACKEND_H

#include <vector>
#include "allegro/palette.h"

struct BITMAP;

class GraphicsBackend
{
public:	
	~GraphicsBackend();
	
	/*
	* Read the configuration options from the config file. The options will be 
	* read from the config section "XXX-graphics" where XXX is replaced
	* by the prefix string.
	*
	* Current options are:
	* - resx; resy: Forces a resolution of resx x resy when the program is run
	*               in windowed mode. The game screen will be stretched to fit
	*               this resolution. Note that resolutions that are not integer
	*               multiples of the "native" resolution of the program (as set
	*               by registerVirtualModes()) is not recommend, as there will
	*               be pixel scaling artifacts.
	* - fullscreen: Whether the program starts off in full screen, or windowed.
	* - fps:        The frame rate of the program, in frames per second. The
	*               waitTick() function will synchronize the program with this
	*               frame rate.
	*/
	void readConfigurationOptions(const std::string &prefix);

	/*
	* Writes the configuration options to the config file. The options will be
	* written to the config section "XXX-graphics" where XXX is replaced by the
	* prefix string.
	*/
	void writeConfigurationOptions(const std::string &prefix);

	/*
	* The program operates on two different screens:
	* - the physical screen, which can be of any size (set by the user);
	* - a virtual screen, whose size is set by the program (so that UI elements
	*   can be positioned manually in favorable locations, etc.)
	* The program is rendered by taking the virtual screen and stretching it
	* onto the physical screen.
	*
	* This function notifies the graphics backend about the requested virtual
	* screen resolutions. When the backend is initialized, the virtual screen
	* will be created at one of these resolutions.
	* Resolutions should be listed in order of most favorable, to least
	* favorable. The first resolution encountered in the list, whose width and
	* height is at most the width and height of the physical screen, will be
	* used.
	*
	* This function *must* be called before calling initialize(). Subsequent
	* calls to registerVirtualModes() overwrite the modes registered earlier.
	* Calling this function after the graphics backend is initialized will
	* force the backend to reset the video mode and recreate the virtual
	* screen (choosing the resolution per the above procedure.)
	*/
	void registerVirtualModes(const std::vector<std::pair<int, int> > &desiredModes);

	/*
	* Registers callback functions that should be invoked when the program
	* gains focus (switchin) or loses focus (switchout). NULL can be used
	* for one or both parameters to indicate that no callback should be
	* invoked.
	*
	* Calling this function multiple times replaces any callbacks registered
	* previously. Can be called after the graphics backend has been initialized.
	*
	* Callbacks *MUST NOT* invoke any GraphicsBackend method, or any Allegro
	* graphics routines: doing so risks crashing or hanging the program!
	*/
	void registerSwitchCallbacks(void(*switchin)(), void(*switchout)());

	/*
	* Initializes the graphics backend, creating the physical and virtual 
	* screens, setting the video, etc.
	*
	* Must be called before any screen can be shown to the user, or any drawing
	* routines are called.
	*
	* Will attempt to create a physical window of size resx x resy (if windowed
	* mode was requested) or a fullscreen window whose resolution matches that
	* of the first requested virtual screen resolution (using 
	* registerVirtualModes()). Will attempt to recover in the case that these
	* resolutions are not available:
	* 1. First, if fullscreen mode was requested but the first registered
	*    virtual screen resolution is not available, all other registered
	*    resolutions will be tried, in order.
	* 2. Then, windowed mode will be tried at rex x resy resolution. If
	*    a windowed screen was requested, initialization skips straight to
	*    this step.
	* 3. Finally, if the previous steps failed, this function will attempt all
	*    resolutions registered using registerVirtualModes(), in order.
	*
	* If the physical screen is initialized successfully, this function next
	* creates a virtual screen at one of the resolutions requested using
	* registerVirtualModes(). Each registered resolution is checked and the
	* first, in order, that fits within the physical screen (has width and
	* height at most that of the physical screen) is chosen.
	*
	* If the physical video mode could not be set, or if no registered
	* virtual screen fits within the physical screen width and height,
	* initialization fails and this function returns false. The program
	* should probably exit at this point: no recovery is possible, and
	* attempting to call any graphics or drawing routine will probably
	* crash the program.
	* Returns true otherwise (on success). Calling this function after
	* the graphics backend is already initialized has no additional effect.
	*/
	bool initialize();

	/*
	* Queries the mode chosen for the virtual screen during initialization.
	* The return value will be an index into the list of desired virtual
	* screen resolutions specified with registerVirtualModes().
	*
	* This method only functions after the graphics backend has been
	* initialized. Returns -1 otherwise.
	*/
	int getVirtualMode() { return curmode_; }

	/*
	* Queries the virtual screen width and height. This resolution will match
	* the getVirtualMode()-th entry passed into registerVirtualModes().
	*
	* Can only be called after the graphics backend has been initialized.
	* Return value is undefined otherwise.
	*/
	int virtualScreenW() { return virtualw_; }
	int virtualScreenH() { return virtualh_; }

	/*
	* Sets the resolution of the physical window to the specified width and
	* height.
	* This resolution is only meaningful in windowed mode, not fullscreen. (See
	* the documentation of initialize() for more details.) Can be called after
	* the graphics backend has been initialized, in which case the physical
	* window is resized immediately (if currently in windowed mode) or the
	* next time the user switches out of fullscreen mode.
	*
	* If the specified resolution is impossible, this function will attempt to
	* fall back to another resolution, using the procedure spelled out in
	* initialize(). If these attempts fail (i.e., if the graphics backend is
	* left in an inconsistent and unitialized state) this function will return
	* false, and the calling program probably needs to terminate.
	* Will return true otherwise (even if the graphics backend is not able
	* to change the window resolution to the desired values.)
	*/
	bool setScreenResolution(int width, int height);
	
	/*
	* Queries the current physical screen width and height.
	* Must be called after the graphics backend has been initialized. Return
	* values are undefined otherwise.
	*/
	int screenW();
	int screenH();	
	
	/*
	* Requests that the physical window should be fullscreen or windowed.
	* This function can be called after the graphics backend has been
	* initialized, in which case the screen mode will immediately change
	* to the desired setting (if it's not already).
	* 
	* The screen resolution and mode will be set according to the procedure
	* described for initialize(), and in particular, the window may not
	* switch to fullscreen if this mode is not supported by the video card.
	* Returns false if the graphics backend cannot fall back to any working
	* video mode (in which case the graphics backend is no longer in a 
	* consistent state and the caller should probably terminate.)
	* Returns true otherwise (even if the graphics backend was not able to
	* switch to the requested mode.)
	*/
	bool setFullscreen(bool fullscreen);

	/*
	* Queries whether the physical window is currently in fullscreen mode, or
	* windowed mode.
	* Can only be called after the graphics backend is initialized. The return
	* value is undefined otherwise.
	*/
	bool isFullscreen() { return fullscreen_; }

	/*
	* Pauses the program, until the next tick of a clock running at fps frames
	* per second. Call this function once per event loop iteration to maintain
	* the program at a constant frame rate.
	* Can only be called after the graphics backend has been initialized.
	* Does nothing otherwise.
	*/
	void waitTick();

	/*
	* Draws the contents of the framebuffer to screen.
	* This function is the *ONLY* time that the screen is refreshed! This function
	* must thus be called at least once within any even loop where the user
	* expects to see changing graphics, a moving mouse pointer, etc.
	*
	* There is some chance that this call can fail (due to the graphics mode
	* no longer being valid, and the failure of the graphics backend to fall
	* back to another graphics mode.) This function returns falls if this
	* happens, in which case the caller should probably terminate. Returns true
	* otherwise.
	*
	* Can only be called once the graphics backend has been initialized. Does
	* nothing otherwise.
	*/
	bool showBackBuffer();

	/*
	* Returns the number of frames rendered (i.e., the number of times
	* showBackBuffer() was called) in the last second. Estimates the
	* actual FPS of the running programming.
	* Can only be called after the graphics bakcend has been initialized.
	* Return value is undefined otherwise.
	*/
	int getLastFPS();

	/*
	* When the graphics backend fails at setting a video mode, and falls
	* back to another video mode, there is a pause. This function sets
	* the length of that pause, in milliseconds.
	* Can be called at any time, but is most useful before intialization,
	* since initialize() might need to try several different modes, depending
	* on the limitations of the user's hardware.
	*/
	void setVideoModeSwitchDelay(int msec);

	/*
	* Converts coordinates from the physical screen to the virtual screen.
	* For example, screenW() maps to virtualScreenW(), etc.
	* Overwrites x and y with the transformed coordaintes.
	* This function can only be called if the graphics backend is initialized.
	* Does nothing otherwise.
	*/
	void physicalToVirtual(int &x, int &y);

	/*
	* Converts coordinates from the virtual screen to the physiscal screen.
	* For example, virtualScreenW() maps to screenW(), etc.
	* Overwrites x and y with the transformed coordaintes.
	* This function can only be called if the graphics backend is initialized.
	* Does nothing otherwise.
	*/
	void virtualToPhysical(int &x, int &y);		

	friend void update_frame_counter();
	friend void onSwitchIn();
	friend void onSwitchOut();
	friend class Backend;

private:
	GraphicsBackend();

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
	
	static int fps_;

	static void(*switch_in_func_)();
	static void(*switch_out_func_)();

	static bool windowsFullscreenFix_;

	static volatile int frame_counter;
	static volatile int frames_this_second;
	static volatile int prev_frames_this_second;
};

#endif