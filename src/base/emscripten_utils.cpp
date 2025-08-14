#include "base/emscripten_utils.h"
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include "base/zc_alleg.h"
#include <allegro5/events.h>
#include <filesystem>

namespace fs = std::filesystem;

EM_ASYNC_JS(void, em_init_fs_, (), {
  // Initialize the filesystem with 0-byte files for every quest.
  const manifest = await ZC.getQuestManifest();

  function writeFakeFile(path, url) {
    url = url || ZC.dataOrigin + path;
    FS.mkdirTree(PATH.dirname(path));
    FS.writeFile(path, '');
    window.ZC.pathToUrl[path] = url;
  }

  for (const [id, quest] of Object.entries(manifest)) {
    const questPrefix = '/' + id;

    for (const music of quest.music) {
        writeFakeFile(questPrefix + '/music/' + music);
    }

    for (const release of quest.releases) {
        const releasePrefix = questPrefix + '/' + release.name;
        for (const resource of release.resources) {
			const path = releasePrefix + '/' + resource;
			let url = ZC.dataOrigin + path;
			if (resource.endsWith('.qst')) url += '.gz';
            writeFakeFile(path, url);
        }
        for (const music of quest.music) {
            writeFakeFile(releasePrefix + '/music/' + music, ZC.dataOrigin + questPrefix + '/music/' + music);
        }
    }
  }

  // Setup the "lazy" files - these are part of the normal package resources, but are not always needed
  // so are excluded from the main data file.
  for (const file of window.ZC_Constants.files) {
    FS.mkdirTree(PATH.dirname(file));
    writeFakeFile(file, '../files' + file);
  }

  await ZC.configureMount();
});
void em_init_fs() {
  em_init_fs_();
}

EM_ASYNC_JS(void, em_sync_fs_, (), {
  await ZC.fsSync(false);
});
void em_sync_fs() {
  em_sync_fs_();
}

// Many files don't have real data until we know the user needs it.
// See em_init_fs
EM_ASYNC_JS(void, em_fetch_file_, (const char *path), {
  try {
    path = UTF8ToString(path);
    if (!path.startsWith('/')) path = '/' + path;
    try {
        if (FS.stat(path).size) return;
    } catch {
    }

	// /host is handled by webserver.mjs, for local development only.
    const url = path.startsWith('/host') ? path : window.ZC.pathToUrl[path];
    if (!url) return;

    const data = await ZC.fetchAsByteArray(url);
	if (!data) {
		// Fetch failed (could be offline).
		console.error(`error fetching ${path}`, e);
		return;
	}

	FS.mkdirTree(PATH.dirname(path));
    FS.writeFile(path, data);
  } catch (e) {
    console.error(`error fetching ${path}`, e);
  }
});
void em_fetch_file(std::string path) {
  em_fetch_file_(path.c_str());
}

// TODO: is this necessary still? Can we just always attempt to fetch a file (em_fetch_file_ handles
// when a file is already present)?
bool em_is_lazy_file(std::string path) {
  path = (fs::current_path() / path).string();

  if (strncmp("/host/", path.c_str(), strlen("/host/")) == 0) {
    return true;
  }

  if (strncmp("/quests/", path.c_str(), strlen("/quests/")) == 0) {
    return true;
  }

  if (strncmp("/tilesets/", path.c_str(), strlen("/tilesets/")) == 0) {
    return true;
  }

  return EM_ASM_INT({
    return ZC_Constants.files.includes(UTF8ToString($0));
  }, path.c_str());
}

std::string get_initial_file_dialog_folder() {
  return "/local/";
}

void em_mark_initializing_status() {
	EM_ASM({
		ZC.setStatus('Initializing Runtime ...');
	});
}

void em_mark_ready_status() {
	EM_ASM({
		ZC.setStatus('Ready');
	});
}

bool em_is_mobile() {
  return EM_ASM_INT({
    return window.matchMedia("(hover: none)").matches;
  });
}

void em_open_link(std::string url) {
  EM_ASM({
    window.open(UTF8ToString($0), '_blank');
  }, url.c_str());
}

void em_open_test_mode(const char* qstpath, int dmap, int screen, int retsquare) {
	EM_ASM({
    ZC.openTestMode(UTF8ToString($0), $1, $2, $3);
	}, qstpath, dmap, screen, retsquare);
}

EM_ASYNC_JS(int, em_compile_zscript_, (const char* script_path, const char* console_path, const char* qr), {
  script_path = UTF8ToString(script_path);
  console_path = UTF8ToString(console_path);
  qr = UTF8ToString(qr);
  const {code} = await ZC.runZscriptCompiler(script_path, console_path, qr);
  return code;
});
int em_compile_zscript(const char* script_path, const char* console_path, const char* qr) {
  return em_compile_zscript_(script_path, console_path, qr);
}

bool has_init_fake_key_events = false;
ALLEGRO_EVENT_SOURCE fake_src;
extern "C" void create_synthetic_key_event(ALLEGRO_EVENT_TYPE type, int keycode)
{
  if (!has_init_fake_key_events)
  {
    al_init_user_event_source(&fake_src);
    all_keyboard_queue_register_event_source(&fake_src);
    has_init_fake_key_events = true;
  }

  ALLEGRO_EVENT event;
  event.any.type = type;
  event.keyboard.keycode = keycode;
  al_emit_user_event(&fake_src, &event, NULL);
}
