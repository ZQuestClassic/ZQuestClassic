import { fileSave, fileOpen, directoryOpen } from "browser-fs-access";
import * as kv from "idb-keyval";
import { createElement, fsReadAllFiles } from "./utils.js";

/** @type {FileSystemDirectoryHandle} */
let attachedDirHandle = null;

export function getAttachedDirHandle() {
  return attachedDirHandle;
}

let configuredMountPromiseResolve;
const configuredMountPromise = new Promise(resolve => {
  configuredMountPromiseResolve = resolve;
});

export async function configureMount() {
  let type = new URLSearchParams(location.search).get('storage');
  if (type !== 'fs' && type !== 'idb') type = undefined;

  if (type === undefined) {
    attachedDirHandle = await kv.get('attached-dir');
    if (!self.showDirectoryPicker || attachedDirHandle === false || !await requestPermission()) {
      type = 'idb';
      attachedDirHandle = null;
    } else {
      type = 'fs';
    }
  }

  // Mount the persisted files (saves folder and zc.cfg live here).
  FS.mkdirTree('/local');
  if (type === 'fs') {
    await setAttachedDir(attachedDirHandle);
  } else {
    FS.mount(IDBFS, {}, '/local');
  }
  await ZC.fsSync(true);
  if (!FS.analyzePath('/local/zc.cfg').exists) {
    FS.writeFile('/local/zc.cfg', FS.readFile('/zc_web.cfg'));
  }
  if (!FS.analyzePath('/local/zquest.cfg').exists) {
    FS.writeFile('/local/zquest.cfg', FS.readFile('/zquest_web.cfg'));
  }
  await renderSettingsPanel();
  configuredMountPromiseResolve();
}

export async function requestPermission() {
  const options = { mode: 'readwrite' };

  if (attachedDirHandle) {
    if (await attachedDirHandle.queryPermission(options) === 'granted') return true;

    try {
      if (await attachedDirHandle.requestPermission(options) !== 'granted') {
        return false;
      }
      return true;
    } catch {
      // ...
    }
  }

  // User activation is required to request permissions
  document.querySelector('.content').classList.add('hidden');
  document.querySelector('.permission').classList.remove('hidden');
  document.querySelector('.permission .folder-name').textContent = attachedDirHandle?.name;
  document.querySelector('.permission .already-attached').classList.toggle('hidden', !attachedDirHandle);
  document.querySelector('.permission .not-attached').classList.toggle('hidden', !!attachedDirHandle);

  let result = await new Promise((resolve) => {
    document.querySelector('.permission .ok').addEventListener('click', () => {
      resolve(true);
    });
    document.querySelector('.permission .cancel').addEventListener('click', () => {
      kv.set('attached-dir', false);
      resolve(false);
    });
  });

  if (result && !attachedDirHandle) {
    try {
      attachedDirHandle = await self.showDirectoryPicker({
        id: 'zc',
      });
    } catch { }
  }
  if (result && attachedDirHandle) result = await attachedDirHandle.requestPermission(options) === 'granted';
  document.querySelector('.content').classList.remove('hidden');
  document.querySelector('.permission').classList.add('hidden');
  return result;
}

async function setAttachedDir(dirHandle) {
  try {
    FS.unmount('/local');
  } catch { }

  attachedDirHandle = dirHandle;
  if (dirHandle) {
    await kv.set('attached-dir', dirHandle);
  } else {
    await kv.del('attached-dir');
  }

  if (!dirHandle) return;

  await import('./fsfs.js');
  FS.mount(FSFS, { dirHandle: attachedDirHandle }, '/local');
  await new Promise((resolve, reject) => FS.syncfs(true, (err) => {
    if (err) return reject(err);
    resolve();
  }));
}

export function setupSettingsPanel() {
  const el = document.querySelector('.settings');

  const filesEl = el.querySelector('.settings__browser-files');
  filesEl.addEventListener('click', async (e) => {
    const fileEl = e.target.closest('[data-path]');
    if (!fileEl) return;

    const path = fileEl.getAttribute('data-path');
    const blob = new Blob([FS.readFile(path)]);
    await fileSave(blob, {
      fileName: path.replace('/local/', ''),
    });
  });

  const criticalFileNames = ['zc.cfg', 'zquest.cfg', 'zc.sav'];

  async function handleFilesCopy(files) {
    // Just in case user is faster than the game.
    await configuredMountPromise;

    for (const file of files) {
      let localRelPath;
      if (file.webkitRelativePath) {
        localRelPath = [
          ...PATH.dirname(file.webkitRelativePath).split('/').slice(1),
          PATH.basename(file.webkitRelativePath),
        ].join('/');
      } else {
        localRelPath = file.name;
      }

      const path = '/local/' + localRelPath;

      // Backup existing files.
      if (FS.analyzePath(path).exists) {
        const backupPath = '/local/.backup/' + localRelPath;
        FS.mkdirTree(PATH.dirname(backupPath));
        FS.writeFile(backupPath, FS.readFile(path));
      }

      FS.mkdirTree(PATH.dirname(path));
      FS.writeFile(path, new Uint8Array(await file.arrayBuffer()));
    }

    await ZC.fsSync(false);
    if (files.some(file => criticalFileNames.includes(file.name))) window.location.reload();
    await renderSettingsPanel();
  }

  el.querySelector('.settings__copy button.copy-folder').addEventListener('click', async () => {
    const files = await directoryOpen({ recursive: true });
    await handleFilesCopy(files);
  });
  el.querySelector('.settings__copy button.copy-file').addEventListener('click', async () => {
    const file = await fileOpen();
    await handleFilesCopy([file]);
  });

  el.querySelector('.settings__attach button.attach').addEventListener('click', async () => {
    const dir = await self.showDirectoryPicker({
      id: 'zc',
    });
    await setAttachedDir(dir);
    window.location.reload();
  });
  el.querySelector('.settings__attach button.unattach').addEventListener('click', async () => {
    await setAttachedDir(null);
    window.location.reload();
  });
}

export async function renderSettingsPanel() {
  const el = document.querySelector('.settings');
  const shouldUseDirPicker = !!self.showDirectoryPicker && attachedDirHandle !== false;

  {
    el.querySelector('.settings__attach').classList.toggle('hidden', !shouldUseDirPicker);

    const attachEl = el.querySelector('.settings__attach button.attach');
    attachEl.classList.toggle('hidden', attachedDirHandle);

    const unattachEl = el.querySelector('.settings__attach button.unattach');
    unattachEl.classList.toggle('hidden', !attachedDirHandle);
    unattachEl.textContent = `Folder attached: ${attachedDirHandle?.name}–Click to unattach`;
  }

  el.querySelector('.settings__copy').classList.toggle('hidden', shouldUseDirPicker);

  if (!shouldUseDirPicker) {
    const filesEl = el.querySelector('.settings__browser-files');
    filesEl.innerHTML = '';
    for (const { path, timestamp } of fsReadAllFiles('/local')) {
      const fileEl = createElement('div', 'file');
      fileEl.append(createElement('div', '', path.replace('/local/', '')));
      fileEl.append(createElement('div', '', new Date(timestamp).toLocaleString()));
      fileEl.setAttribute('data-path', path);
      filesEl.append(fileEl);
    }
  }

  el.querySelector('.settings__download').classList.toggle('hidden', !!attachedDirHandle);

  {
    const cfgs = [
      'zc.cfg',
      'oot.cfg',
      '2MGM.cfg',
      'ultra.cfg',
      'ppl160.cfg',
      'freepats.cfg',
    ];
    let currentCfg = await kv.get('timidity-cfg');
    if (!currentCfg || !FS.analyzePath(`/etc/${currentCfg}`).exists) {
      currentCfg = cfgs[0];
    }

    const cfgsSelectEl = el.querySelector('.settings__timidity-cfgs select');
    cfgsSelectEl.textContent = '';
    for (const cfg of cfgs) {
      const el = createElement('option', '', cfg);
      cfgsSelectEl.append(el);
    }
    cfgsSelectEl.addEventListener('change', async () => {
      await kv.set('timidity-cfg', cfgsSelectEl.value)
    });
    cfgsSelectEl.value = currentCfg;
    FS.writeFile('/etc/timidity.cfg', FS.readFile(`/etc/${currentCfg}`));
  }
}
