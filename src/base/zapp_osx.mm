#import <Cocoa/Cocoa.h>

// The HiDPI scale factor (1.0 or 2.0) of the main screen. Used to size a window
// before any display exists.
float zapp_osx_get_main_screen_scale_factor(void)
{
	NSScreen* screen = [NSScreen mainScreen];
	if (screen && [screen respondsToSelector:@selector(backingScaleFactor)])
		return (float)[screen backingScaleFactor];
	return 1.0f;
}

// The usable area of the main screen (excludes the menu bar and dock), in
// physical pixels, so we can size a window that fits fully on-screen.
void zapp_osx_get_main_screen_usable_size(int* w, int* h)
{
	NSScreen* screen = [NSScreen mainScreen];
	float scale = 1.0f;
	if (screen && [screen respondsToSelector:@selector(backingScaleFactor)])
		scale = [screen backingScaleFactor];
	NSRect vf = screen ? [screen visibleFrame] : NSMakeRect(0, 0, 0, 0);
	if (w) *w = (int)(vf.size.width * scale);
	if (h) *h = (int)(vf.size.height * scale);
}
