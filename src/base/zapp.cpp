#include "zapp.h"
#include "zc_alleg.h"
#include <filesystem>
#include <string>
#ifdef __APPLE__
#include <unistd.h>
#endif

#ifdef HAS_SENTRY
#define SENTRY_BUILD_STATIC 1
#include "sentry.h"

void sentry_atexit()
{
    sentry_close();
}
#endif

static App app_id = App::undefined;

bool is_in_osx_application_bundle()
{
#ifdef __APPLE__
    return std::filesystem::current_path().string().find("/ZeldaClassic.app/") != std::string::npos;
#else
    return false;
#endif
}

void common_main_setup(App id, int argc, char **argv)
{
    app_id = id;

#ifdef HAS_SENTRY
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://133f371c936a4bc4bddec532b1d1304a@o1313474.ingest.sentry.io/6563738");
    sentry_options_set_release(options, "zelda-classic@" SENTRY_RELEASE_TAG);
    sentry_options_set_handler_path(options, "crashpad_handler.exe");
    sentry_init(options);
    sentry_set_tag("app", ZC_APP_NAME);
    atexit(sentry_atexit);
#endif

    // This allows for opening a binary from Finder and having ZC be in its expected
    // working directory (the same as the binary). Only used when not inside an application bundle,
    // and only for testing purposes really. See comment about `SKIP_APP_BUNDLE` in buildpack_osx.sh
#ifdef __APPLE__
    if (!is_in_osx_application_bundle() && argc > 0) {
        chdir(std::filesystem::path(argv[0]).parent_path().c_str());
    }
#endif
}

App get_app_id()
{
    return app_id;
}

static ALLEGRO_EVENT_QUEUE *evq = nullptr;
void zc_install_mouse_event_handler()
{
	if (!evq)
	{
		evq = al_create_event_queue();
		al_register_event_source(evq, al_get_mouse_event_source());
	}
}
void zc_process_mouse_events()
{
	if (!evq)
		return;

	ALLEGRO_EVENT event;
	while (al_get_next_event(evq, &event))
	{
		switch (event.type)
		{
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			int mouse_x = event.mouse.x;
			int mouse_y = event.mouse.y;

			int native_width, native_height, display_width, display_height, offset_x, offset_y;
			double scale;
			all_get_display_transform(&native_width, &native_height, &display_width, &display_height, &offset_x, &offset_y, &scale);

			// Show OS cursor when hovering over letterboxing.
			if (mouse_on_screen() && !(gfx_capabilities & GFX_HW_CURSOR))
			{
				static bool is_within_letterbox_prev = !false;
				bool is_within_letterbox = !all_get_fullscreen_flag() &&
					(mouse_x < offset_x || mouse_y < offset_y ||
					mouse_x >= display_width - offset_x || mouse_y >= display_height - offset_y);
				
				if (is_within_letterbox_prev == is_within_letterbox)
				{
					break;
				}
				else if (is_within_letterbox)
				{
					enable_hardware_cursor();
					if (app_id == App::zquest)
						show_mouse(NULL);
				}
				else
				{
					disable_hardware_cursor();
					if (app_id == App::zquest)
						show_mouse(screen);
				}
				is_within_letterbox_prev = is_within_letterbox;
			}
			break;
		}
		}
	}
}
