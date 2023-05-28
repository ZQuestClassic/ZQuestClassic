#include "base/emscripten_utils.h"
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include "base/zc_alleg.h"
#include <allegro5/events.h>

EM_ASYNC_JS(void, em_init_fs_, (), {
  // Initialize the filesystem with 0-byte files for every quest.
  const quests = await ZC.fetch("https://hoten.cc/quest-maker/play/quest-manifest.json").catch(error => {
	console.error(error.toString());
	return [];
  });
  FS.mkdir('/_quests');

  function writeFakeFile(path, url) {
    FS.mkdirTree(PATH.dirname(path));
    FS.writeFile(path, '');
    // UHHHH why does this result in an error during linking (acorn parse error) ???
    // window.ZC.pathToUrl[path] = `https://hoten.cc/quest-maker/play/${url}`;
    window.ZC.pathToUrl[path] = url;
  }

  for (let i = 0; i < quests.length; i++) {
    const quest = quests[i];
    if (!quest.urls.length) continue;

    for (const url of quest.urls) {
      const path = window.ZC.createPathFromUrl(url);
      writeFakeFile(path, 'https://hoten.cc/quest-maker/play/' + url);
    }
    for (const extraResourceUrl of quest.extraResources || []) {
      writeFakeFile(window.ZC.createPathFromUrl(extraResourceUrl), 'https://hoten.cc/quest-maker/play/' + extraResourceUrl);
    }
  }

  for (const file of window.ZC_Constants.files) {
    FS.mkdirTree(PATH.dirname(file));
    writeFakeFile(file, 'files' + file);
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

// Quest files don't have real data until we know the user needs it.
// See em_init_fs
EM_ASYNC_JS(void, em_fetch_file_, (const char *path), {
  try {
    path = UTF8ToString(path);
    if (FS.stat(path).size) return;

    const url = window.ZC.pathToUrl[path];
    if (!url) return;

    const data = await ZC.fetchAsByteArray(url);
    FS.writeFile(path, data);
  } catch (e) {
    // Fetch failed (could be offline) or path did not exist.
    console.error(`error loading ${path}`, e);
  }
});
void em_fetch_file(const char *path) {
  em_fetch_file_(path);
}

bool em_is_lazy_file(const char *path) {
  if (strncmp("/_quests/", path, strlen("/_quests/")) == 0) {
    return true;
  }

  return EM_ASM_INT({
    return ZC_Constants.files.includes(UTF8ToString($0));
  }, path);
}

std::string get_initial_file_dialog_folder() {
  return "/local/";
}

EM_ASYNC_JS(emscripten::EM_VAL, get_query_params_, (), {
  const params = new URLSearchParams(location.search);
  return Emval.toHandle({
    quest: params.get('quest') || '',
  });
});
QueryParams get_query_params() {
  auto val = emscripten::val::take_ownership(get_query_params_());
  QueryParams result;
  result.quest = val["quest"].as<std::string>();
  return result;
}

void em_mark_initializing_status() {
	EM_ASM({
		Module.setStatus('Initializing Runtime ...');
	});
}

void em_mark_ready_status() {
	EM_ASM({
		Module.setStatus('Ready');
	});
}

bool em_is_mobile() {
  return EM_ASM_INT({
    return window.matchMedia("(hover: none)").matches;
  });
}

void em_open_test_mode(const char* qstpath, int dmap, int scr, int retsquare) {
	EM_ASM({
    ZC.openTestMode(UTF8ToString($0), $1, $2, $3);
	}, qstpath, dmap, scr, retsquare);
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
