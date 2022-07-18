#include "base/zapp.h"
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
