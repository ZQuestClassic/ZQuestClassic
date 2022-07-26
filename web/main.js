import { renderSettingsPanel, setupSettingsPanel } from "./settings.js";
import { createUrlString, ensureFolderExists, fetchWithProgress, mkdirp } from "./utils.js";

window.ZC = {
  pathToUrl: {},
  createPathFromUrl(url) {
    const urlSplit = url.split('/');
    const filename = urlSplit[urlSplit.length - 1];
    const path = `/_quests/${filename}`;
    return path;
  },
  async fetch(url, opts) {
    const formatKB = (bytes) => `${Math.floor(bytes / 1024)} KB`;
    const response = await fetchWithProgress(url, opts, (received, total, done) => {
      if (done) {
        Module.setStatus('Ready');
      } else if (received && total) {
        Module.setStatus(`Fetching file (${Math.floor(received / 1024)}/${formatKB(total)})`, received / total);
      } else if (received) {
        Module.setStatus(`Fetching file (${formatKB(received)})`);
      } else {
        Module.setStatus('Fetching file');
      }
    });

    const contentType = await response.headers.get('Content-Type');
    if (contentType === 'application/json') {
      const json = new TextDecoder('utf-8').decode(await response.arrayBuffer());
      return JSON.parse(json);
    }

    return response;
  },
  async fetchAsByteArray(url, opts) {
    const response = await ZC.fetch(url, opts);
    return new Uint8Array(await response.arrayBuffer());
  },
  mkdirp,
  ensureFolderExists,
  url: '',
  setShareableUrl(params) {
    ZC.url = createUrlString('', params);
  },
  openTestMode(quest, dmap, screen, retsquare) {
    if (0x80 < screen) return;

    const params = { quest, dmap, screen };
    if (retsquare !== -1) params.retsquare = retsquare;
    const url = createUrlString(ZC_Constants.zeldaUrl, params);
    window.open(url, '_blank');
  },
  async fsSync() {
    // Sync /local/browser to IndexedDB.
    await new Promise(resolve => FS.syncfs(false, resolve));

    // Sync /local/filesystem to attached folder.
    // TODO
  }
};

async function main() {
  try {
    const statusElement = document.getElementById('status');
    const progressElement = document.getElementById('progress');

    const args = [];
    const params = new URLSearchParams(location.search);

    let questPath = params.get('quest');
    if (questPath) {
      questPath = questPath.startsWith('local/') ?
        questPath :
        `/_quests/${questPath}`;

      if (TARGET === 'zelda') {
        if (params.get('dmap') && params.get('screen')) {
          args.push('-test', questPath, params.get('dmap'), params.get('screen'));
          if (params.get('retsquare')) args.push(params.get('retsquare'));
          document.querySelector('.button--testmode').classList.remove('hidden');
        }
      } else if (TARGET === 'zquest') {
        args.push(questPath);
      }
    }

    // window.Module = await initModule({
    window.Module = {
      arguments: args,
      canvas: document.querySelector('canvas'),
      instantiateWasm,
      onRuntimeInitialized: () => {
        setupTouchControls();
        setupCopyUrl();
        setupSettingsPanel();
        if (TARGET === 'zquest') setupOpenTestMode();
      },
      setStatus: function (text, percentProgress = null) {
        // `.data.js` emscripten generated script passes progress like this:
        //   Downloading data... (x/y)
        // And there's no way to configure that, so just parse it.
        if (text.startsWith('Downloading data... (')) {
          const [, x, y] = text.match(/(\d+)\/(\d+)/);
          text = `Downloading data... (${Math.floor(x / 1024)}/${Math.floor(y / 1024)} KB)`;
          percentProgress = x / y;
        }

        if (!text || text === 'Running...') return;
        if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };

        if (text === 'Ready') {
          progressElement.value = null;
          progressElement.max = null;
          progressElement.hidden = true;
          statusElement.hidden = true;
          return;
        }

        statusElement.hidden = false;

        if (text === Module.setStatus.last.text) return;
        const now = Date.now();
        if (percentProgress && now - Module.setStatus.last.time < 30) return; // if this is a progress update, skip it if too soon
        Module.setStatus.last.time = now;
        Module.setStatus.last.text = text;
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
      expectedDataFileDownloads: 0,
      totalDependencies: 0,
      monitorRunDependencies: function (left) {
        Module.totalDependencies = Math.max(Module.totalDependencies, left);
        if (Module.totalDependencies === 1) return;

        const progress = (Module.totalDependencies - left) / (Module.totalDependencies);
        Module.setStatus(left ? `Preparing... (${Module.totalDependencies - left}/${Module.totalDependencies})` : '', progress);
      }
    };

    window.addEventListener('resize', resize);
    resize();

    const requestPersist = () => {
      navigator.storage.persist();
      canvas.removeEventListener('click', requestPersist);
    }
    canvas.addEventListener('click', requestPersist);

    for (const el of [...document.querySelectorAll('.panel-button[data-panel]')]) {
      el.addEventListener('click', () => {
        for (const buttonEl of document.querySelectorAll('.panel-button')) {
          const panelEl = document.querySelector(buttonEl.getAttribute('data-panel'));
          if (el === buttonEl) {
            buttonEl.classList.toggle('active');
            if (panelEl) panelEl.classList.toggle('hidden');
            if (panelEl && buttonEl.getAttribute('data-panel') === '.settings') renderSettingsPanel();
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

    await renderQuestList();
  } catch (err) {
    console.error(err);
    document.querySelector('.content').textContent = err.toString();
  }
}

// This is exactly the code from Emscripten's `instantiateWasm` except it uses
// ZC.fetch to provide progress updates when downloading the wasm file.
function instantiateWasm(info, receiveInstance) {
  function instantiateArrayBuffer(receiver) {
    return getBinaryPromise().then(function (binary) {
      return WebAssembly.instantiate(binary, info);
    }).then(function (instance) {
      return instance;
    }).then(receiver, function (reason) {
      err("failed to asynchronously prepare wasm: " + reason);
      if (isFileURI(wasmBinaryFile)) {
        err("warning: Loading from a file URI (" + wasmBinaryFile + ") is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing");
      }
      abort(reason);
    });
  }

  function receiveInstantiationResult(result) {
    receiveInstance(result["instance"], result["module"]);
  }

  if (!wasmBinary && typeof WebAssembly.instantiateStreaming == "function" && !isDataURI(wasmBinaryFile) && !isFileURI(wasmBinaryFile) && typeof fetch == "function") {
    return ZC.fetch(wasmBinaryFile, {
      credentials: "same-origin"
    }).then(function (response) {
      var result = WebAssembly.instantiateStreaming(response, info);
      return result.then(receiveInstantiationResult, function (reason) {
        err("wasm streaming compile failed: " + reason);
        err("falling back to ArrayBuffer instantiation");
        return instantiateArrayBuffer(receiveInstantiationResult);
      });
    });
  } else {
    return instantiateArrayBuffer(receiveInstantiationResult);
  }
}

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

  const response = await fetch("https://hoten.cc/quest-maker/play/quest-manifest.json");
  const quests = await response.json();

  for (const quest of quests) {
    if (!quest.urls.length) continue;

    const el = document.createElement('div');
    el.classList.add('quest-list__entry');
    el.setAttribute('data-quest-index', quests.indexOf(quest));
    el.quest = quest;
    el.append(quest.name);
    questListEntriesEl.append(el);
  }

  function showQuest(quest) {
    document.querySelector('.quest-list__entry.selected')?.classList.remove('selected');
    document.querySelector(`.quest-list__entry[data-quest-index="${quests.indexOf(quest)}"]`).classList.add('selected');

    const contentEl = questListCurrentTemplate.content.cloneNode(true);
    contentEl.querySelector('.name').textContent = quest.name;
    contentEl.querySelector('.author').textContent = quest.author;
    contentEl.querySelector('.genre').textContent = quest.genre;
    contentEl.querySelector('.version').textContent = quest.zcVersion;
    contentEl.querySelector('.purezc-link').href = quest.projectUrl;

    if (quest.rating) {
      contentEl.querySelector('.rating').textContent = quest.rating.score;
      contentEl.querySelector('.rating-votes').textContent = quest.rating.distribution.reduce((a, b) => a + b, 0);
    } else {
      contentEl.querySelector('._rating').remove();
    }

    contentEl.querySelector('.information').innerHTML = quest.informationHtml;
    contentEl.querySelector('.description').innerHTML = quest.descriptionHtml;
    contentEl.querySelector('.story').innerHTML = quest.storyHtml;
    contentEl.querySelector('.tips').innerHTML = quest.tipsAndCheatsHtml;
    contentEl.querySelector('.credits').innerHTML = quest.storyHtml;

    const imagesEl = contentEl.querySelector('.images');
    for (const imgUrl of quest.imageUrls) {
      const imgEl = document.createElement('img');
      imgEl.src = 'https://hoten.cc/quest-maker/play/' + imgUrl;
      imagesEl.append(imgEl);
    }

    const url = quest.urls[0];
    const path = window.ZC.createPathFromUrl(url);
    const questParamValue = path.replace('/_quests/', '');

    const playQuestUrl = new URL(ZC_Constants.zeldaUrl, location.href);
    playQuestUrl.search = '';
    playQuestUrl.searchParams.set('quest', questParamValue);
    contentEl.querySelector('.play-link').href = playQuestUrl;

    const editQuestUrl = new URL(ZC_Constants.zquestUrl, location.href);
    editQuestUrl.search = '';
    editQuestUrl.searchParams.set('quest', questParamValue);
    contentEl.querySelector('.editor-link').href = editQuestUrl;

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
  const initialSelectedQuest =
    (qsQuest && quests.find(q => window.ZC.createPathFromUrl(q.urls[0]).replace('/_quests/', '') === qsQuest)) || quests[0];
  showQuest(initialSelectedQuest);
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

  const getShareableUrl = Module.cwrap('get_shareable_url', 'string', []);
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

main();
