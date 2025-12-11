#include "base/zapp.h"
#include "base/zc_alleg.h"
#include "zconfig.h"
#include "base/zsys.h"
#include <filesystem>
#include <string>

#ifdef _WIN32
#include <algorithm>
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifdef ALLEGRO_MACOSX
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
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

static std::function<void()> crash_cb;

static sentry_value_t on_crash_callback(const sentry_ucontext_t *uctx, sentry_value_t event, void *closure)
{
	(void)uctx;
	(void)closure;

	if (crash_cb)
		crash_cb();

	return event;
}
#endif

namespace fs = std::filesystem;

static App app_id = App::undefined;

// No trailing slash.
static std::string get_exe_folder_path()
{
	std::string path;

#ifdef _WIN32
	wchar_t wc[260] = {0};
	GetModuleFileNameW(NULL, wc, 260);
	std::wstring ws(wc);
	std::transform(ws.begin(), ws.end(), std::back_inserter(path), [](wchar_t c) { return (char)c; });
	std::replace(path.begin(), path.end(), '\\', '/');
#elif defined(ALLEGRO_MACOSX)
	char buf[PATH_MAX] = {0};
	uint32_t length = PATH_MAX;
	if (!_NSGetExecutablePath(buf, &length))
		path = std::string(buf, length>0 ? length : 0);
#elif defined(__EMSCRIPTEN__)
	path = "";
#else
	char c[260] = {0};
	int length = (int)readlink("/proc/self/exe", c, 260);
	path = std::string(c, length>0 ? length : 0);
#endif

	return path.substr(0, path.rfind('/'));
}

static bool is_exe_in_bin_folder()
{
	return fs::path(zapp_get_exe_folder_path()).filename() == "bin";
}

void zapp_set_crash_cb(std::function<void()> cb)
{
#ifdef HAS_SENTRY
	crash_cb = cb;
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

	bool disable_chdir = std::getenv("ZC_DISABLE_OSX_CHDIR") != nullptr || std::getenv("ZC_DISABLE_CHDIR") != nullptr;
	if (!disable_chdir)
	{
#ifdef ALLEGRO_LINUX
		// Change to share/zquestclassic folder.
		if (is_exe_in_bin_folder())
		{
			auto share_path = fs::path(zapp_get_exe_folder_path()) / "../share/zquestclassic";
			if (fs::exists(share_path))
				chdir(share_path.c_str());
		}
#else
		std::string new_cwd = zapp_get_exe_folder_path();
		if (fs::current_path().string() != new_cwd)
			chdir(new_cwd.c_str());
#endif
	}

#ifdef HAS_SENTRY
	// Clean up the last crash screenshot.
	const char* crash_screenshot_path = ".sentry_native/screenshot.png";
	{
		std::error_code ec;
		if (fs::exists(crash_screenshot_path, ec))
			fs::remove(crash_screenshot_path, ec);
	}

    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://133f371c936a4bc4bddec532b1d1304a@o1313474.ingest.sentry.io/6563738");
    sentry_options_set_database_path(options, ".sentry_native");
    sentry_options_set_release(options, "zelda-classic@" RELEASE_TAG);
    sentry_options_add_attachment(options, crash_screenshot_path);
    sentry_options_set_on_crash(options, on_crash_callback, NULL);
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

#ifdef ALLEGRO_SDL
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

	// The updater has to move some actively used files to a temporary location, but cannot delete them itself.
	auto update_active_files = fs::path(".updater-active-files");
	std::error_code ec;
	fs::remove_all(update_active_files, ec);
}

std::string zapp_get_exe_folder_path()
{
	static std::string folder_path = get_exe_folder_path();
	return folder_path;
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

// If (saved_width, saved_height) is >0, ensures that fits in the primary monitor. If neither are true, fall
// back to default.
// Default will scale up (base_width, base_height) by an integer amount to fill up the primary monitor
// as much as possible, up to 3x.
std::pair<int, int> zc_get_default_display_size(int base_width, int base_height, int saved_width, int saved_height, int max_scale)
{
	ALLEGRO_MONITOR_INFO info;
	al_get_monitor_info(0, &info);
	int mw = info.x2 - info.x1;
	int mh = info.y2 - info.y1;
	if (saved_width > 0 && saved_height > 0 && saved_width <= mw && saved_height <= mh)
	{
		return {saved_width, saved_height};
	}

#ifdef ALLEGRO_MACOSX
	// https://talk.automators.fm/t/getting-screen-dimensions-while-accounting-the-menu-bar-dock-and-multiple-displays/13639
	mh -= 38;
#endif
#ifdef _WIN32
	// Title bar.
	mh -= 23;
#endif
	// Small buffer, so the default window is never as big as the monitor.
	mw -= 50;
	mh -= 50;

	int s = std::min(mh / base_height, mw / base_width);
	s = std::min(max_scale, s);
	int w = base_width * s;
	int h = base_height * s;
	return {w, h};
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
