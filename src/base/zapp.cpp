#include "base/zapp.h"
#include "base/zc_alleg.h"
#include "zconfig.h"
#include "base/zsys.h"
#include <filesystem>
#include <string>

#ifdef _WIN32
#include <allegro5/allegro_windows.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#ifdef ALLEGRO_SDL
#include <SDL.h>
#endif

#ifdef HAS_SENTRY
#define SENTRY_BUILD_STATIC 1
#include "sentry.h"

void sentry_atexit()
{
    sentry_close();
}
#endif

namespace fs = std::filesystem;

static App app_id = App::undefined;

bool is_in_osx_application_bundle()
{
#ifdef __APPLE__
    return std::filesystem::current_path().string().find(".app/") != std::string::npos;
#else
    return false;
#endif
}

// TODO: move qst.cpp to base/
int32_t get_qst_buffers();

void common_main_setup(App id, int argc, char **argv)
{
    app_id = id;

	if (std::getenv("ZC_HEADLESS") != nullptr)
	{
		set_headless_mode();
	}

#ifdef HAS_SENTRY
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://133f371c936a4bc4bddec532b1d1304a@o1313474.ingest.sentry.io/6563738");
    sentry_options_set_release(options, "zelda-classic@" RELEASE_TAG);
	// Only track sessions for the main apps.
	if (id != App::zelda && id != App::zquest)
		sentry_options_set_auto_session_tracking(options, 0);
    sentry_init(options);
    switch (id)
    {
        case App::zelda:
            sentry_set_tag("app", "zelda");
            break;
        case App::zquest:
            sentry_set_tag("app", "zquest");
            break;
        case App::zscript:
            sentry_set_tag("app", "zscript");
            break;
        case App::launcher:
            sentry_set_tag("app", "launcher");
            break;
		case App::updater:
            sentry_set_tag("app", "updater");
            break;
    }
    atexit(sentry_atexit);

	// Sentry backend is configured to scrub the IP. This just gets us a rough unique user count.
	sentry_value_t user = sentry_value_new_object();
	sentry_value_set_by_key(user, "ip_address", sentry_value_new_string("{{auto}}"));
	sentry_set_user(user);
#endif

    // This allows for opening a binary from Finder and having ZC be in its expected
    // working directory (the same as the binary). Only used when not inside an application bundle,
    // and only for testing purposes really. See comment about `SKIP_APP_BUNDLE` in package_mac.sh
#ifdef __APPLE__
    if (!is_in_osx_application_bundle() && argc > 0) {
        chdir(std::filesystem::path(argv[0]).parent_path().c_str());
    }
#endif

#ifdef ALLEGRO_SDL
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

	// The updater has to move some actively used files to a temporary location, but cannot delete them itself.
	auto update_active_files = fs::path(".updater-active-files");
	std::error_code ec;
	fs::remove_all(update_active_files, ec);
	if (fs::exists("zquest.exe"))
		fs::remove("zquest.exe", ec);
}

App get_app_id()
{
    return app_id;
}

bool is_web()
{
#ifdef __EMSCRIPTEN__
	return true;
#else
	return false;
#endif
}

bool is_ci()
{
	// Cache the result, so it is not possible to ever change the result of "is_ci()".
	static bool state = std::getenv("CI") != nullptr;
	return state;
}

static bool headless;

void set_headless_mode()
{
	headless = true;
}

// Note: we don't truly have a headless mode (yet), but this function is useful for skipping GUI prompts for CI and scripted
// use cases.
bool is_headless()
{
	return headless;
}

// https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
// https://mariusbancila.ro/blog/2021/05/19/how-to-build-high-dpi-aware-native-desktop-applications/
// On windows, Allegro sets DPI awareness at runtime:
//   https://github.com/liballeg/allegro5/blob/0d9271d381c33ab1096b424beadfd82050aaa2d3/src/win/wsystem.c#L142
// But it's better to do it in the application manifest, hence `set_property(TARGET zelda PROPERTY VS_DPI_AWARE "PerMonitor")`
double zc_get_monitor_scale()
{
#ifdef __EMSCRIPTEN__
	return 1.0;
#endif
	if(zc_get_config("gui","ignore_monitor_scale",1))
		return 1.0;
#ifdef _WIN32
	if (all_get_display())
	{
		// GetDpiForWindow only works for Windows 10 and greater.
		// why not use al_get_monitor_dpi? Because allegro uses GetDpiForMonitor,
		// which is an older API.
		HMODULE user32 = LoadLibraryW(L"user32");

		typedef UINT (WINAPI *GetDpiForWindowPROC)(HWND hwnd);
		GetDpiForWindowPROC imp_GetDpiForWindow =
			(GetDpiForWindowPROC) GetProcAddress(user32, "GetDpiForWindow");
		if (user32 && user32 != INVALID_HANDLE_VALUE) {
			HWND hwnd = al_get_win_window_handle(all_get_display());
			int dpi = imp_GetDpiForWindow(hwnd);
			FreeLibrary(user32);
			return dpi / 96.0;
		}

		return al_get_monitor_dpi(0) / 96.0;
	}

	HDC hdc = GetDC(NULL);
	int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(NULL, hdc);
	return dpi / 96.0;
#else
	return al_get_monitor_dpi(0) / 96.0;
#endif
}

// If (saved_width, saved_height) is not -1, ensures that fits in the primary monitor. If neither are true, fall
// back to default.
// Default will scale up (base_width, base_height) by an integer amount to fill up the monitor
// as much as possible.
// NOTE: On Windows, instead the inputs are expected to be "monitor-scale-independent" (a made up concept), are
// multiplied by a scaling factor based on the monitor's DPI.
std::pair<int, int> zc_get_default_display_size(int base_width, int base_height, int saved_width, int saved_height)
{
	ALLEGRO_MONITOR_INFO info;
	al_get_monitor_info(0, &info);
	int mw = info.x2 - info.x1;
	int mh = info.y2 - info.y1;

#ifdef ALLEGRO_MACOSX
	// https://talk.automators.fm/t/getting-screen-dimensions-while-accounting-the-menu-bar-dock-and-multiple-displays/13639
	mh -= 38;
#endif

#ifdef _WIN32
	double monitor_scale = zc_get_monitor_scale();
	if (saved_width != -1 && saved_height != -1 && saved_width * monitor_scale <= mw && saved_height * monitor_scale <= mh)
	{
		return {saved_width * monitor_scale, saved_height * monitor_scale};
	}
	return {base_width * monitor_scale, base_height * monitor_scale};
#else
	// I've confirmed this is the desired behavior on my Mac. Still need to test on Windows,
	// especially w/ its OS-level DPI scaling feature, so for now Windows will use the previous hacky method above.
	if (saved_width != -1 && saved_height != -1 && saved_width <= mw && saved_height <= mh)
	{
		return {saved_width, saved_height};
	}
	int s = std::min(mh / base_height, mw / base_width);
	int w = base_width * s;
	int h = base_height * s;
	return {w, h};
#endif
}

extern bool DragAspect;
extern double aspect_ratio;
static void doAspectResize()
{
	if (!DragAspect || all_get_fullscreen_flag())
		return;

	static int prev_width = 0, prev_height = 0;
	
	if (prev_width == 0 || prev_height == 0)
	{
		prev_width = al_get_display_width(all_get_display());
		prev_height = al_get_display_height(all_get_display());
	}

	if (prev_width != al_get_display_width(all_get_display()) || prev_height != al_get_display_height(all_get_display()))
	{
		bool width_first = true;
		
		if (abs(prev_width - al_get_display_width(all_get_display())) < abs(prev_height - al_get_display_height(all_get_display()))) width_first = false;
		
		if (width_first)
		{
			al_resize_display(all_get_display(), al_get_display_width(all_get_display()), al_get_display_width(all_get_display())*aspect_ratio);
		}
		else
		{
			al_resize_display(all_get_display(), al_get_display_height(all_get_display())/aspect_ratio, al_get_display_height(all_get_display()));
		}
	}

	prev_width = al_get_display_width(all_get_display());
	prev_height = al_get_display_height(all_get_display());
}

extern int window_min_width, window_min_height;
void zc_do_minsize()
{
	if(all_get_fullscreen_flag()) return;
	if(!(window_min_width || window_min_height)) return;
	
	int wid = al_get_display_width(all_get_display());
	int hei = al_get_display_height(all_get_display());
	if(wid < window_min_width || hei < window_min_height)
	{
		if(wid < window_min_width) wid = window_min_width;
		if(hei < window_min_height) hei = window_min_height;
		al_resize_display(all_get_display(),wid,hei);
	}
}

void zc_process_display_events()
{
	all_process_display_events();
	// TODO: should do this only in response to a resize event
	doAspectResize();
	zc_do_minsize();
}

void zapp_reporting_add_breadcrumb(const char* category, const char* message)
{
#ifdef HAS_SENTRY
	sentry_value_t crumb = sentry_value_new_breadcrumb("info", message);
	sentry_value_set_by_key(crumb, "category", sentry_value_new_string(category));
	sentry_add_breadcrumb(crumb);
#endif
}

void zapp_reporting_set_tag(const char* key, const char* value)
{
#ifdef HAS_SENTRY
	sentry_set_tag(key, value);
#endif
}

void zapp_reporting_set_tag(const char* key, int value)
{
#ifdef HAS_SENTRY
	sentry_set_tag(key, std::to_string(value).c_str());
#endif
}
