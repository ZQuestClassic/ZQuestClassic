#include "emscripten_utils.h"
#include <emscripten/emscripten.h>

EM_ASYNC_JS(void, init_fs_em_, (), {
  // Initialize the filesystem with 0-byte files for every quest.
  const response = await fetch("https://hoten.cc/quest-maker/play/quest-manifest.json");
  const quests = await response.json();
  FS.mkdir('/_quests');

  window.ZC = {
    pathToUrl: {},
  };
  for (let i = 0; i < quests.length; i++) {
    const quest = quests[i];
    if (!quest.urls.length) continue;

    const url = quest.urls[0];
    const id = quest.projectUrl ?
      quest.projectUrl.substring('https://www.purezc.net/index.php?page=quests&id='.length) :
      0;
    const urlSplit = url.split('/');
    const filename = urlSplit[urlSplit.length - 1];
    const path = `/_quests/${id}-${filename}`;
    FS.writeFile(path, '');
    // UHHHH why does this result in an error during linking (acorn parse error) ???
    // window.ZC.pathToUrl[path] = `https://hoten.cc/quest-maker/play/${url}`;
    window.ZC.pathToUrl[path] = 'https://hoten.cc/quest-maker/play/' + url;
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
EM_ASYNC_JS(void, fetch_quest_em_, (const char *path), {
  path = UTF8ToString(path);
  if (FS.stat(path).size) return;

  const url = window.ZC.pathToUrl[path];
  const response = await fetch(url);
  const data = await response.arrayBuffer();
  const buffer = new Uint8Array(data);
  FS.writeFile(path, buffer);
});
void fetch_quest_em(const char *path) {
  fetch_quest_em_(path);
}
