import { configureMount, renderSettingsPanel, setupSettingsPanel } from "./settings.js";
import { createElement, createUrlString, fetchWithProgress, formatBytes, fsReadAllFiles } from "./utils.js";
import { handleFileLaunch } from "./file_launch.js";
import { compileScriptWasmModule, createScriptWasmHandle, pollScriptWasmHandle, destroyScriptWasmHandle, destroyAllScriptWasmHandles } from "./zasm.js";

globalThis.ZC = {
  // TODO: this should be https://data.zquestclassic.com , but having issues with SSL/X-Amz-Meta-Inflated-Content-Size
  dataOrigin: 'https://zc-data.nyc3.digitaloceanspaces.com',
  pathToUrl: {},
  setStatus: function (text, percentProgress = null) {
    // `.data.js` emscripten generated script passes progress like this:
    //   Downloading data... (x/y)
    // And there's no way to configure that, so just parse it.
    if (text.startsWith('Downloading data... (')) {
      const [, x, y] = text.match(/(\d+)\/(\d+)/);
      text = `Downloading data... (${formatBytes(x, y, 'MB')})`;
      percentProgress = x / y;
    }

    if (!text || text === 'Running...') return;
    if (!ZC.setStatus.last) ZC.setStatus.last = { time: Date.now(), text: '' };

    const statusElement = document.getElementById('status');
    const progressElement = document.getElementById('progress');

    if (text === 'Ready') {
      progressElement.value = null;
      progressElement.max = null;
      progressElement.hidden = true;
      statusElement.hidden = true;
      return;
    }

    statusElement.hidden = false;

    if (text === ZC.setStatus.last.text) return;
    const now = Date.now();
    if (percentProgress && now - ZC.setStatus.last.time < 30) return; // if this is a progress update, skip it if too soon
    ZC.setStatus.last.time = now;
    ZC.setStatus.last.text = text;
    if (percentProgress !== null && percentProgress > 0) {
      progressElement.value = percentProgress;
      progressElement.max = 1;
      progressElement.hidden = false;
    } else {
      progressElement.value = null;
      progressElement.max = null;
      progressElement.hidden = true;
    }
    statusElement.innerHTML = text;
  },
  async fetch(url, opts) {
    const response = await fetchWithProgress(url, opts, (received, total, done) => {
      if (done) {
        ZC.setStatus('Ready');
      } else if (received && total) {
        ZC.setStatus(`Fetching file (${formatBytes(received, total)})`, received / total);
      } else if (received) {
        ZC.setStatus(`Fetching file (${formatBytes(received)})`);
      } else {
        ZC.setStatus('Fetching file');
      }
    });

    const contentType = response.headers.get('Content-Type');
    if (contentType === 'application/json' || url.endsWith('.json')) {
      const json = new TextDecoder('utf-8').decode(await response.arrayBuffer());
      return JSON.parse(json);
    }

    return response;
  },
  async getQuestManifest() {
    if (ZC.questManifestPromise) {
      return ZC.questManifestPromise;
    }

    ZC.questManifestPromise = ZC.fetch(ZC.dataOrigin + '/manifest.json').catch(error => {
      console.error(error.toString());
      return [];
    });
    return ZC.questManifestPromise;
  },
  async fetchAsByteArray(url, opts) {
    const response = await ZC.fetch(url, opts);
    if (!response.ok) {
      console.error('bad response for url:', url, response);
      return null;
    }

    return new Uint8Array(await response.arrayBuffer());
  },
  url: '',
  setShareableUrl(params) {
    ZC.url = createUrlString('', params);
  },
  openTestMode(quest, dmap, screen, retsquare) {
    if (0x80 < screen) return;

    const params = { test: quest, dmap, screen };
    if (retsquare !== -1) params.retsquare = retsquare;
    const url = createUrlString(ZC_Constants.zeldaUrl, params);
    window.open(url, '_blank');
  },
  async runZscriptCompiler(scriptPath, consolePath, qr) {
    scriptPath = PATH.join('/root-fs', scriptPath);
    consolePath = PATH.join('/root-fs', consolePath);

    const zscriptPath = './zscript.mjs';
    const {default: ZScript} = await import(zscriptPath);
    let onExitPromiseResolve;
    const onExitPromise = new Promise(resolve => onExitPromiseResolve = resolve);
    await ZScript({
      preRun(zscript) {
        // For some reason `set_config_file` errors if the file doesn't exist ...
        if (!FS.analyzePath('/local/zscript.cfg').exists) FS.writeFile('/local/zscript.cfg', '');

        zscript.FS.mkdir('/root-fs');
        zscript.FS.mount(PROXYFS, {
          root: '/',
          fs: FS,
        }, '/root-fs');
        zscript.FS.chdir('/root-fs');
      },
      onExit: onExitPromiseResolve,
      arguments: ['-linked', '-input', scriptPath, '-console', consolePath, '-qr', qr],
    });

    const code = await onExitPromise;
    // Not necessary, but avoids lag when playing the sfx.
    await new Promise(resolve => setTimeout(resolve, 100));
    return {code};
  },
  async fsSync(populate) {
    // Sync /local to IndexedDB or to the attached folder.
    await new Promise((resolve, reject) => FS.syncfs(populate, (err) => {
      if (err) return reject(err);
      resolve();
    }));
  },
  configureMount,
  fsReadAllFiles,
  compileScriptWasmModule,
  createScriptWasmHandle,
  pollScriptWasmHandle,
  destroyScriptWasmHandle,
  destroyAllScriptWasmHandles,
};

async function main() {
  window.Module = window.Module || {};
  Module.noInitialRun = true;

  let readyToRunPromiseResolve;
  const readyToRunPromise = new Promise(resolve => readyToRunPromiseResolve = resolve);

  // window.Module = await initModule({
  window.Module = Object.assign(Module, {
    arguments: [],
    canvas: document.querySelector('canvas'),
    // instantiateWasm,
    onRuntimeInitialized: () => {
      if (TARGET === 'zplayer') setupTouchControls();
      else {
        // TODO: better way to hide touch controls (just don't render them?)
        for (const el of [...document.querySelectorAll('.touch-inputs')]) {
          el.classList.toggle('hidden', true);
        }
      }
      setupCopyUrl();
      setupSettingsPanel();
      if (TARGET === 'zeditor') setupOpenTestMode();
      readyToRunPromiseResolve();
    },
    setStatus: function (text, percentProgress = null) {
      ZC.setStatus(text, percentProgress);
    },
    expectedDataFileDownloads: 0,
    totalDependencies: 0,
    monitorRunDependencies: function (left) {
      Module.totalDependencies = Math.max(Module.totalDependencies, left);
      if (Module.totalDependencies === 1) return;

      const progress = (Module.totalDependencies - left) / (Module.totalDependencies);
      ZC.setStatus(left ? `Preparing... (${Module.totalDependencies - left}/${Module.totalDependencies})` : '', progress);
    },
    onExit: function (code) {
      const msg = `exit with code: ${code}`;
      if (code === 0) {
        console.log(msg);
      } else {
        console.error(msg);
      }
    },
    preRun: [() => {
      if (IS_CI) {
        ENV.CI = '1';
        console.warn('CI mode enabled');
      }
    }],
    locateFile: function (file, base) {
      return new URL(file, base || rootUrl).href;
    },
  });

  const params = new URLSearchParams(location.search);
  const args = Module.arguments;

  let openPath = params.get('open');
  if (openPath) {
    const manifest = await ZC.getQuestManifest();
    openPath = openPath.startsWith('/') ? openPath : `/${openPath}`;
    const m = (openPath.startsWith('/quests/') || openPath.startsWith('/tilesets/')) && openPath.match(/\/([^/]*\/[^/]*\/[^/]*)\/?(.*)?/);
    if (m) {
      const [, id, file] = m;
      if (!file && manifest[id]) {
        openPath = `/${manifest[id].defaultPath}`;
      }
    }
  }

  if (openPath) {
    if (TARGET === 'zeditor') args.push(openPath);
    else if (TARGET === 'zplayer') args.push('-web-open', openPath);
  }

  if (TARGET === 'zplayer') {
    const testPath = params.get('test');
    const screen = params.get('screen');
    const dmap = params.get('dmap');
    const retsquare = params.get('retsquare');
    if (testPath && screen && dmap) {
      args.push('-test', testPath, dmap, screen);
      if (retsquare) args.push(retsquare);
      document.querySelector('.button--testmode').classList.remove('hidden');
    }
  }

  for (const [key, value] of params.entries()) {
    if (['test', 'screen', 'dmap', 'retsquare', 'open'].includes(key)) {
      continue;
    }

    const cliSwitch = '-' + key.replace(/[A-Z]/g, (match, offset) => (offset > 0 ? '-' : '') + match.toLowerCase());
    args.push(cliSwitch);
    if (value !== '') args.push(value);
  }

  window.addEventListener('resize', resize);
  resize();

  const requestPersist = () => {
    navigator.storage.persist();
    canvas.removeEventListener('click', requestPersist);
  }
  canvas.addEventListener('click', requestPersist);

  for (const el of [...document.querySelectorAll('.panel-button[data-panel]')]) {
    el.addEventListener('click', async () => {
      for (const buttonEl of document.querySelectorAll('.panel-button')) {
        const panelEl = document.querySelector(buttonEl.getAttribute('data-panel'));
        if (el === buttonEl) {
          buttonEl.classList.toggle('active');
          if (panelEl) panelEl.classList.toggle('hidden');
          if (panelEl && buttonEl.getAttribute('data-panel') === '.settings') await renderSettingsPanel();
        } else {
          buttonEl.classList.remove('active');
          if (panelEl) panelEl.classList.add('hidden');
        }
      }

      const canvasFocus = !document.querySelector('.panel-button.active');
      document.body.classList.toggle('canvas-focus', canvasFocus);
      document.querySelector('.content').classList.toggle('hidden', !canvasFocus);
      if (canvasFocus) {
        resize();
        document.body.scrollIntoView();
      }
    });
  }
  document.body.classList.toggle('canvas-focus', true);

  await readyToRunPromise;
  callMain(Module.arguments);

  await renderQuestList();

  if (params.has('launch')) {
    const result = await handleFileLaunch();
    if (result.openInEditor) {
      window.location.href = createUrlString(ZC_Constants.zquestUrl, { open: result.quest });
    } else {
      window.location.href = createUrlString(ZC_Constants.zeldaUrl, { open: result.quest });
    }
  }
}

// This is exactly the code from Emscripten's `instantiateWasm` except it uses
// ZC.fetch to provide progress updates when downloading the wasm file.
// TODO: no it doesn't, because of a chrome bug where caching won't work with a wrapper Response object,
// which is used for progress tracking. So for now, this just won't give a progress indicator.
// https://bugs.chromium.org/p/chromium/issues/detail?id=719172#c102
// function instantiateWasm(info, receiveInstance) {
//   function instantiateArrayBuffer(receiver) {
//     return getBinaryPromise().then(function (binary) {
//       return WebAssembly.instantiate(binary, info);
//     }).then(function (instance) {
//       return instance;
//     }).then(receiver, function (reason) {
//       err("failed to asynchronously prepare wasm: " + reason);
//       if (isFileURI(wasmBinaryFile)) {
//         err("warning: Loading from a file URI (" + wasmBinaryFile + ") is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing");
//       }
//       abort(reason);
//     });
//   }

//   function receiveInstantiationResult(result) {
//     receiveInstance(result["instance"], result["module"]);
//   }

//   if (!wasmBinary && typeof WebAssembly.instantiateStreaming == "function" && !isDataURI(wasmBinaryFile) && !isFileURI(wasmBinaryFile) && typeof fetch == "function") {
//     return ZC.fetch(wasmBinaryFile, {
//       credentials: "same-origin"
//     }).then(function (response) {
//       var result = WebAssembly.instantiateStreaming(response, info);
//       return result.then(receiveInstantiationResult, function (reason) {
//         err("wasm streaming compile failed: " + reason);
//         err("falling back to ArrayBuffer instantiation");
//         return instantiateArrayBuffer(receiveInstantiationResult);
//       });
//     });
//   } else {
//     return instantiateArrayBuffer(receiveInstantiationResult);
//   }
// }

function resize() {
  let w = 640;
  let h = 480;
  const heightAvail = innerHeight - document.querySelector('header').clientHeight;
  const canvasScale = Math.min(innerWidth / w, heightAvail / h);
  canvas.style.width = (Math.floor(w * canvasScale)) + "px";
  canvas.style.height = (Math.floor(h * canvasScale)) + "px";
}

async function renderQuestList() {
  const questListEl = document.querySelector('.quest-list');
  const questListEntriesEl = questListEl.querySelector('.quest-list__entries-list');
  const questListCurrentEl = questListEl.querySelector('.quest-list__current');
  const questListCurrentTemplate = document.querySelector('.tmpl-selected-quest');

  const manifest = await ZC.getQuestManifest();
  const quests = Object.values(manifest);

  for (const quest of quests) {
    if (quest.type !== 'quests') continue;
    if (!['auto', true].includes(quest.approval)) continue;

    const el = document.createElement('div');
    el.classList.add('quest-list__entry');
    el.setAttribute('data-quest-index', quests.indexOf(quest));
    el.quest = quest;
    el.append(quest.name);
    questListEntriesEl.append(el);
    el.tabIndex = -1;
  }

  questListEntriesEl.addEventListener('keyup', (e) => {
    if (e.keyCode === 38) {
      const questIndex = Number(e.target.previousElementSibling.getAttribute('data-quest-index'));
      const quest = quests[questIndex];
      showQuest(quest);
      e.target.previousElementSibling.focus();
    } else if (e.keyCode === 40 && e.target.nextElementSibling) {
      const questIndex = Number(e.target.nextElementSibling.getAttribute('data-quest-index'));
      const quest = quests[questIndex];
      showQuest(quest);
      e.target.nextElementSibling.focus();
    }
  });

  function showQuest(quest) {
    const questPrefix = `${ZC.dataOrigin}/${quest.id}`;

    document.querySelector('.quest-list__entry.selected')?.classList.remove('selected');
    document.querySelector(`.quest-list__entry[data-quest-index="${quests.indexOf(quest)}"]`).classList.add('selected');

    const contentEl = questListCurrentTemplate.content.cloneNode(true).children[0];

    function fillOrDelete(selector, value) {
      if (value) {
        contentEl.querySelector(selector).textContent = value;
      } else {
        contentEl.querySelector(selector).parentElement.remove();
      }
    }

    fillOrDelete('.name', quest.name);
    fillOrDelete('.author', quest.authors.map(a => a.name).join(', '));
    fillOrDelete('.genre', quest.genre);
    fillOrDelete('.version', quest.version);

    if (quest.projectUrl) {
      contentEl.querySelector('.purezc-link').href = quest.projectUrl;
    } else {
      contentEl.querySelector('.purezc-link').remove();
    }

    if (quest.rating) {
      contentEl.querySelector('.rating').textContent = quest.rating.score;
      contentEl.querySelector('.rating-votes').textContent = quest.rating.distribution.reduce((a, b) => a + b, 0);
    } else {
      contentEl.querySelector('._rating').remove();
    }

    if (quest.informationHtml) contentEl.querySelector('.information').innerHTML = quest.informationHtml;
    if (quest.descriptionHtml) contentEl.querySelector('.description').innerHTML = quest.descriptionHtml;
    if (quest.storyHtml) contentEl.querySelector('.story').innerHTML = quest.storyHtml;
    if (quest.tipsAndCheatsHtml) contentEl.querySelector('.tips').innerHTML = quest.tipsAndCheatsHtml;
    if (quest.creditsHtml) contentEl.querySelector('.credits').innerHTML = quest.creditsHtml;

    function setCurImage(image) {
      const imgEl = document.createElement('img');
      imgEl.crossOrigin = 'anonymous';
      imgEl.src = `${questPrefix}/${image}`;
      contentEl.querySelector('.current-image').innerHTML = '';
      contentEl.querySelector('.current-image').append(imgEl);
    }

    const imagesEl = contentEl.querySelector('.images');
    if (quest.images.length > 1) {
      for (const image of quest.images) {
        const imgEl = document.createElement('img');
        imgEl.crossOrigin = 'anonymous';
        imgEl.src = `${questPrefix}/${image}`;
        imagesEl.append(imgEl);
        imgEl.addEventListener('click', () => {
          setCurImage(image);
        });
        imgEl.addEventListener('mouseover', () => {
          setCurImage(image);
        });
      }
    }

    if (quest.images.length > 0) setCurImage(quest.images[0]);

    const release = quest.releases[0];
    const qsts = release.resources.filter(r => r.endsWith('.qst'));
    for (const resource of qsts) {
      // Only set `name` query param, which doesn't do anything, when making a link for
      // a quest that only has a single qst file. Otherwise, to disambiguate use the entire
      // file path (which likely makes obvious the name of the quest).
      const openParamValue = qsts.length > 1 ?
        `${quest.id}/${release.name}/${resource}` :
        quest.id;
      const args = {open: openParamValue};
      if (qsts.length === 1) {
        args.name = quest.name.replace(/['!?]/g, '').replace(/[\s:&]+/g, '-').replace(/[-]+/g, '-');
      }

      const playEl = createElement('a', 'play-link');
      playEl.href = createUrlString(ZC_Constants.zeldaUrl, args);
      playEl.textContent = 'Play!';

      const editEl = createElement('a');
      editEl.href = createUrlString(ZC_Constants.zquestUrl, args);
      editEl.textContent = 'Open in Editor';

      const el = createElement('div', 'link-group');
      contentEl.querySelector('.links').append(el);
      el.append(resource, playEl, editEl);
    }

    questListCurrentEl.textContent = '';
    questListCurrentEl.append(contentEl);
  }

  questListEntriesEl.addEventListener('click', (e) => {
    const entryEl = e.target.closest('.quest-list__entry');
    if (!entryEl) return;

    const questIndex = Number(entryEl.getAttribute('data-quest-index'));
    const quest = quests[questIndex];
    showQuest(quest);
  });

  for (const el of document.querySelectorAll('.quest-list__sort-option')) {
    el.addEventListener('click', () => {
      const field = el.textContent;

      let sortBy;
      if (field === 'Name') {
        sortBy = (a, b) => {
          return a.textContent.localeCompare(b.textContent);
        };
      } else if (field === 'Rating') {
        sortBy = (a, b) => {
          const score_a = a.quest.rating?.score || 0;
          const score_b = b.quest.rating?.score || 0;
          return score_b - score_a;
        };
      }

      [...questListEntriesEl.children]
        .sort(sortBy)
        .forEach(node => questListEntriesEl.appendChild(node));
    });
  }

  const qsQuest = new URLSearchParams(location.search).get('quest');
  const qsQuestId = qsQuest ? qsQuest.match(/(quests\/purezc\/\d+)/)?.[0] : '';
  const initialSelectedQuest = (qsQuestId && quests.find(q => q.id == qsQuestId)) || quests.find(q => ['auto', true].includes(q.approval));
  if (initialSelectedQuest) showQuest(initialSelectedQuest);
}

function setupTouchControls() {
  const createKeyEvent = Module.cwrap('create_synthetic_key_event', null, ['int', 'int']);
  const EVENTS = {
    KEY_DOWN: 10,
    KEY_CHAR: 11,
    KEY_UP: 12,
  };
  const KEY_CODES = {
    B: 26,
    A: 24,
    Start: 67,
    Map: 75,
    L: 1,
    R: 19,
    Left: 82,
    Right: 83,
    Down: 84,
    Up: 85,
  };
  const getDirectionalKey = (x, y) => {
    if (x === -1) return KEY_CODES.Left;
    if (x === 1) return KEY_CODES.Right;
    if (y === -1) return KEY_CODES.Down;
    if (y === 1) return KEY_CODES.Up;
  }
  const sendKeyEvent = (eventType, key) => {
    if (eventType === 'touchstart') createKeyEvent(EVENTS.KEY_DOWN, key);
    if (eventType === 'touchmove' || eventType === 'touchstart') createKeyEvent(EVENTS.KEY_CHAR, key);
    if (eventType === 'touchend') createKeyEvent(EVENTS.KEY_UP, key);
  };

  const getKeysForInputElement = (el) => {
    const action = el.getAttribute('data-action');
    if (action) {
      return [KEY_CODES[action]];
    }

    const keys = [];
    const x = Number(el.getAttribute('data-x'));
    const y = Number(el.getAttribute('data-y'));
    if (x) keys.push(getDirectionalKey(x, 0));
    if (y) keys.push(getDirectionalKey(0, y));

    return keys;
  }

  for (const touchInputsEl of document.querySelectorAll('.touch-inputs')) {
    let activeInputEl = null;
    const setActiveInputEl = el => {
      if (activeInputEl) activeInputEl.classList.remove('pressed');
      activeInputEl = el;
      if (activeInputEl) activeInputEl.classList.add('pressed');
    };

    const touchFn = (eventType, e) => {
      let touch;
      if (eventType === 'touchend') {
        if (e.targetTouches.length !== 0) return;
        touch = e.changedTouches[0];
      } else {
        if (e.targetTouches.length !== 1) return;
        touch = e.targetTouches[0];
      }

      const elementMouseIsOver = document.elementFromPoint(touch.clientX, touch.clientY);
      const inputEl = elementMouseIsOver.classList.contains('touch-input') ? elementMouseIsOver : null;

      if (!inputEl && elementMouseIsOver.closest('.touch-inputs') === touchInputsEl && eventType !== 'touchend') {
        // In the "dead zone" between the inputs (ex: the middle of the dpad).
        return;
      }

      if (activeInputEl && (inputEl !== activeInputEl || !inputEl)) {
        for (const key of getKeysForInputElement(activeInputEl)) {
          sendKeyEvent('touchend', key);
        }
      }

      if (!inputEl) {
        setActiveInputEl(null);
        return;
      }

      for (const key of getKeysForInputElement(inputEl)) {
        sendKeyEvent(eventType, key);
      }

      setActiveInputEl(eventType === 'touchend' ? null : inputEl);
    };

    touchInputsEl.addEventListener('touchstart', e => touchFn('touchstart', e));
    touchInputsEl.addEventListener('touchmove', e => touchFn('touchmove', e));
    touchInputsEl.addEventListener('touchend', e => touchFn('touchend', e));
    touchInputsEl.addEventListener('touchcancel', e => touchFn('touchend', e));
    touchInputsEl.addEventListener('contextmenu', e => e.preventDefault());
  }
}

function setupCopyUrl() {
  const el = document.querySelector('.button--copyurl');
  el.classList.remove('hidden');

  // TODO: this should just return the string.
  const getShareableUrl = Module.cwrap('get_shareable_url', 'undefined', []);
  el.addEventListener('click', () => {
    getShareableUrl();
    if (ZC.url) navigator.clipboard.writeText(ZC.url);
  });
}

function setupOpenTestMode() {
  const el = document.querySelector('.button--open-testmode');
  el.classList.remove('hidden');

  const openTestMode = Module.cwrap('open_test_mode', null, []);
  el.addEventListener('click', () => {
    openTestMode();
  });
}

function checkForGamepads() {
  const hasGamepad = navigator.getGamepads().some(Boolean);
  for (const el of [...document.querySelectorAll('.touch-inputs')]) {
    el.classList.toggle('hidden', hasGamepad);
  }
}

function goFullscreen() {
  document.querySelector('main').requestFullscreen();
}

// Don't run this in the workers.
if (typeof window !== 'undefined' && TARGET !== 'playground') {
  main().catch(err => {
    console.error(err);
    document.querySelector('.content').textContent = err.toString();
  });
  checkForGamepads();
  window.addEventListener('gamepadconnected', checkForGamepads);
  window.addEventListener('gamepaddisconnected', checkForGamepads);

  const fullscreenEl = document.createElement('button');
  fullscreenEl.textContent = 'Fullscreen';
  fullscreenEl.classList.add('panel-button');
  fullscreenEl.addEventListener('click', goFullscreen);
  document.addEventListener('DOMContentLoaded', () => {
    document.querySelector('.panel-buttons').append(fullscreenEl);
  });

  // If using gamepad, probably want to go fullscreen.
  // TODO https://bugs.chromium.org/p/chromium/issues/detail?id=1413066
  // if (TARGET === 'zplayer') {
  //   window.addEventListener('gamepadconnected', goFullscreen, { once: true });
  // }
}
