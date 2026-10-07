#include "base/zapp.h"

#include "base/about.h"
#include "base/version.h"
#include "fmt/ranges.h"
#include <filesystem>
#include <optional>
#include <string>
#include <fmt/format.h>

#ifdef _WIN32
#include <algorithm>
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <allegro5/allegro_windows.h>
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
    #define strcasecmp _stricmp
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifndef _WIN32
#include <limits.h>
#include <stdlib.h>
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

static int argc;
static char** argv;
static App app_id = App::undefined;

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
// Best-effort path to the running executable from argv[0], for systems that have no API
// for it (OpenBSD) or where procfs is unavailable. Like a shell, a bare command name is
// looked up in PATH.
static std::string resolve_argv0_path()
{
	if (!argv || !argv[0] || !argv[0][0])
		return "";

	char resolved[PATH_MAX];
	std::string arg0 = argv[0];
	if (arg0.find('/') != std::string::npos)
		return realpath(arg0.c_str(), resolved) ? resolved : "";

	const char* path_env = std::getenv("PATH");
	if (!path_env)
		return "";

	std::string dirs = path_env;
	size_t start = 0;
	while (start <= dirs.size())
	{
		size_t end = dirs.find(':', start);
		if (end == std::string::npos)
			end = dirs.size();
		std::string dir = dirs.substr(start, end - start);
		if (dir.empty())
			dir = ".";
		std::string candidate = dir + "/" + arg0;
		if (access(candidate.c_str(), X_OK) == 0 && realpath(candidate.c_str(), resolved))
			return resolved;
		start = end + 1;
	}
	return "";
}
#endif

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
#elif defined(__APPLE__)
	char buf[PATH_MAX] = {0};
	uint32_t length = PATH_MAX;
	if (!_NSGetExecutablePath(buf, &length))
		path = std::string(buf, length>0 ? length : 0);
#elif defined(__EMSCRIPTEN__)
	path = "";
#else
#if defined(__FreeBSD__) || defined(__DragonFly__)
	// procfs is not mounted by default on the BSDs.
	char buf[PATH_MAX] = {0};
	size_t length = sizeof(buf);
	int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
	if (sysctl(mib, 4, buf, &length, NULL, 0) == 0 && length > 0)
		path = std::string(buf);
#elif defined(__NetBSD__)
	char buf[PATH_MAX] = {0};
	size_t length = sizeof(buf);
	int mib[4] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
	if (sysctl(mib, 4, buf, &length, NULL, 0) == 0 && length > 0)
		path = std::string(buf);
#elif defined(__sun)
	char buf[PATH_MAX] = {0};
	ssize_t length = readlink("/proc/self/path/a.out", buf, sizeof(buf) - 1);
	if (length > 0)
		path = std::string(buf, length);
#elif defined(__linux__)
	char buf[PATH_MAX] = {0};
	ssize_t length = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (length > 0)
		path = std::string(buf, length);
#endif
	if (path.empty())
		path = resolve_argv0_path();
#endif

	return path.substr(0, path.rfind('/'));
}

static bool is_exe_in_bin_folder()
{
	return fs::path(zapp_get_exe_folder_path()).filename() == "bin";
}

void zapp_set_crash_cb([[maybe_unused]] std::function<void()> cb)
{
#ifdef HAS_SENTRY
	crash_cb = cb;
#endif
}

void common_main_setup(App id, int argc_, char **argv_)
{
	argc = argc_;
	argv = argv_;
    app_id = id;

	if (used_switch(argc, argv, "-version"))
	{
		printf("version %s\n", getVersionString());
		zapp_exit(0);
	}

	if (used_switch(argc, argv, "-channel"))
	{
		printf("channel %s\n", getReleaseChannel());
		zapp_exit(0);
	}

	if (used_switch(argc, argv, "-repo"))
	{
		printf("repo %s\n", getRepo());
		zapp_exit(0);
	}

	bool disable_chdir = std::getenv("ZC_DISABLE_OSX_CHDIR") != nullptr || std::getenv("ZC_DISABLE_CHDIR") != nullptr;
	if (!disable_chdir)
	{
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
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
    sentry_options_set_release(options, fmt::format("zelda-classic@{}", getVersionString()).c_str());
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

	// Helps to test crash reporting.
	if (zapp_check_switch("-crash"))
	{
		abort();
	}

	// The updater has to move some actively used files to a temporary location, but cannot delete them itself.
	auto update_active_files = fs::path(".updater-active-files");
	std::error_code ec;
	fs::remove_all(update_active_files, ec);
}

// Note: prefer zapp_check_switch, zapp_get_arg_string, etc.
// TODO: remove.
int32_t used_switch(int32_t argc,char *argv[],const char *s)
{
	// assumes a switch won't be in argv[0]
	for(int32_t i=1; i<argc; i++)
		if(strcasecmp(argv[i],s)==0)
			return i;
			
	return 0;
}

int32_t zapp_check_switch(const char *s, std::vector<const char*> arg_names)
{
    int index = 0;

    for (int i = 1; i < argc; i++)
	{
        if (strcasecmp(argv[i], s) == 0)
		{
			index = i;
			break;
		}
	}

	if (index)
	{
		bool bad_args = false;
		size_t num_args = arg_names.size();
		if (index + num_args >= argc)
		{
			bad_args = true;
		}
		else for (int i = 0; i < num_args; i++)
		{
			const char* arg = argv[index + i + 1];
			if (strlen(arg) == 0 || arg[0] == '-')
			{
				bad_args = true;
				break;
			}
		}

		if (bad_args)
		{
			printf("%s\n", fmt::format("expected switch {} to have {} args: {}", s, num_args, fmt::join(arg_names, ", ")).c_str());
			zapp_exit(1);
		}
	}

    return index;
}

// TODO: remove.
int zapp_get_argc()
{
	return argc;
}

// TODO: remove.
char** zapp_get_argv()
{
	return argv;
}

void zapp_replace_args(int argc_, char **argv_)
{
	argc = argc_;
	argv = argv_;
}

int32_t zapp_get_arg_int(int index)
{
	return std::stoll(argv[index]);
}

std::string zapp_get_arg_string(int index)
{
	return argv[index];
}

std::string zapp_get_exe_folder_path()
{
	static std::string folder_path = get_exe_folder_path();
	return folder_path;
}

std::optional<bool> get_flag_bool(const char* name)
{
	int arg = used_switch(argc, argv, name);
	if (arg) return true;

	std::string no_name = fmt::format("-no{}", name);
	arg = used_switch(argc, argv, no_name.c_str());
	if (arg) return false;

	return std::nullopt;
}

std::optional<int64_t> get_flag_int(const char* name)
{
	int arg = used_switch(argc, argv, name);
	if (arg == 0) return std::nullopt;
	return std::stoll(argv[arg + 1]);
}

std::optional<std::string> get_flag_string(const char* name)
{
	int arg = used_switch(argc, argv, name);
	if (arg == 0) return std::nullopt;
	return argv[arg + 1];
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

void zapp_reporting_add_breadcrumb([[maybe_unused]] const char* category, [[maybe_unused]] const char* message)
{
#ifdef HAS_SENTRY
	sentry_value_t crumb = sentry_value_new_breadcrumb("info", message);
	sentry_value_set_by_key(crumb, "category", sentry_value_new_string(category));
	sentry_add_breadcrumb(crumb);
#endif
}

void zapp_reporting_set_tag([[maybe_unused]] const char* key, [[maybe_unused]] const char* value)
{
#ifdef HAS_SENTRY
	sentry_set_tag(key, value);
#endif
}

void zapp_reporting_set_tag([[maybe_unused]] const char* key, [[maybe_unused]] int value)
{
#ifdef HAS_SENTRY
	sentry_set_tag(key, std::to_string(value).c_str());
#endif
}

static bool g_is_exiting;

bool is_exiting()
{
	return g_is_exiting;
}

void set_is_exiting()
{
	g_is_exiting = true;
}

void zapp_exit(int code)
{
	set_is_exiting();
	exit(code);
}
