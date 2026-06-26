#import <Cocoa/Cocoa.h>
#include "allegro5/display.h"

#include <allegro5/allegro_osx.h>

void zalleg_osx_bring_window_to_foreground(ALLEGRO_DISPLAY* display)
{
	NSWindow* window = (NSWindow*)al_osx_get_window(display);
	dispatch_async(dispatch_get_main_queue(), ^{
		[window makeKeyAndOrderFront:nil];
	});
}

// The HiDPI scale factor (1.0 or 2.0) of the main screen. Used to size a window
// before any display exists.
float zalleg_osx_get_main_screen_scale_factor(void)
{
	NSScreen* screen = [NSScreen mainScreen];
	if (screen && [screen respondsToSelector:@selector(backingScaleFactor)])
		return (float)[screen backingScaleFactor];
	return 1.0f;
}

// The usable area of the main screen (excludes the menu bar and dock), in
// physical pixels, so we can size a window that fits fully on-screen.
void zalleg_osx_get_main_screen_usable_size(int* w, int* h)
{
	NSScreen* screen = [NSScreen mainScreen];
	float scale = 1.0f;
	if (screen && [screen respondsToSelector:@selector(backingScaleFactor)])
		scale = [screen backingScaleFactor];
	NSRect vf = screen ? [screen visibleFrame] : NSMakeRect(0, 0, 0, 0);
	if (w) *w = (int)(vf.size.width * scale);
	if (h) *h = (int)(vf.size.height * scale);
}

// The HiDPI scale factor of the monitor a given display currently lives on.
// Reflects the new monitor after the window is dragged between displays.
float zalleg_osx_get_display_scale_factor(ALLEGRO_DISPLAY* display)
{
	NSWindow* window = (NSWindow*)al_osx_get_window(display);
	if (window && [window respondsToSelector:@selector(backingScaleFactor)])
		return (float)[window backingScaleFactor];
	return 1.0f;
}
