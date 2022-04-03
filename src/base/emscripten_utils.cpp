#include "emscripten_utils.h"
#include <emscripten/emscripten.h>
#include <emscripten/val.h>

EM_ASYNC_JS(void, init_fs_em_, (), {
  // Initialize the filesystem with 0-byte files for every quest.
  const response = await fetch("https://hoten.cc/quest-maker/play/quest-manifest.json");
  const quests = await response.json();
  FS.mkdir('/_quests');

  function writeFakeFile(path, url) {
    FS.writeFile(path, '');
    // UHHHH why does this result in an error during linking (acorn parse error) ???
    // window.ZC.pathToUrl[path] = `https://hoten.cc/quest-maker/play/${url}`;
    window.ZC.pathToUrl[path] = 'https://hoten.cc/quest-maker/play/' + url;
  }

  for (let i = 0; i < quests.length; i++) {
    const quest = quests[i];
    if (!quest.urls.length) continue;

    const url = quest.urls[0];
    const path = window.ZC.createPathFromUrl(url);
    writeFakeFile(path, url);
    for (const extraResourceUrl of quest.extraResources || []) {
      writeFakeFile(window.ZC.createPathFromUrl(extraResourceUrl), extraResourceUrl);
    }
  }

  // Mount the persisted files (zc.sav and zc.cfg live here).
  FS.mkdir('/persist');
  FS.mount(IDBFS, {}, '/persist');
  await new Promise(resolve => FS.syncfs(true, resolve));
  if (!FS.analyzePath('/persist/zc.cfg').exists) {
    FS.writeFile('/persist/zc.cfg', FS.readFile('/zc.cfg'));
  } else {
  }
});
void init_fs_em() {
  init_fs_em_();
}

EM_ASYNC_JS(void, sync_fs_em_, (), {
  await new Promise(resolve => FS.syncfs(false, resolve));
});
void sync_fs_em() {
  sync_fs_em_();
}

// Quest files don't have real data until we know the user needs it.
// See init_fs_em
EM_ASYNC_JS(void, fetch_file_em_, (const char *path), {
  path = UTF8ToString(path);
  if (FS.stat(path).size) return;

  const url = window.ZC.pathToUrl[path];
  if (!url) return;

  const response = await fetch(url);
  const data = await response.arrayBuffer();
  const buffer = new Uint8Array(data);
  FS.writeFile(path, buffer);
});
void fetch_file_em(const char *path) {
  fetch_file_em_(path);
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
