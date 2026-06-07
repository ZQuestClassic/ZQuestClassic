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
