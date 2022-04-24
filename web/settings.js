import { fileSave } from "browser-fs-access";
import * as kv from "idb-keyval";
import { createElement, fsReadAllFiles } from "./utils.js";

/** @type {FileSystemDirectoryHandle} */
let attachedDirHandle = null;

async function requestPermission(dirHandle) {
  const options = { mode: 'readwrite' };
  if (await dirHandle.queryPermission(options) === 'granted') return true;

  try {
    if (await dirHandle.requestPermission(options) !== 'granted') {
      return false;
    }
    return true;
  } catch {
    // ...
  }

  // User activation is required to request permissions
  document.querySelector('.content').classList.add('hidden');
  document.querySelector('.permission').classList.remove('hidden');
  document.querySelector('.permission .folder-name').textContent = dirHandle.name;

  let result = await new Promise((resolve) => {
    document.querySelector('.permission .ok').addEventListener('click', () => {
      resolve(true);
    });
    document.querySelector('.permission .cancel').addEventListener('click', () => {
      resolve(false);
    });
  });


  if (result) result = await dirHandle.requestPermission(options) === 'granted';
  document.querySelector('.content').classList.remove('hidden');
  document.querySelector('.permission').classList.add('hidden');
  return result;
}

async function setAttachedDir(dirHandle) {
  if (dirHandle && !await requestPermission(dirHandle)) {
    dirHandle = null;
  }

  if (attachedDirHandle) FS.unmount('/local/filesystem');
  attachedDirHandle = dirHandle;
  if (dirHandle) {
    await kv.set('attached-dir', dirHandle);
  } else {
    await kv.del('attached-dir');
  }

  renderSettingsPanel();

  if (!dirHandle) return;

  await import('./fsfs.js');
  FS.mkdirTree('/local/filesystem');
  FS.mount(FSFS, { dirHandle: attachedDirHandle }, '/local/filesystem');
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
      fileName: path.replace('/local/browser/', ''),
    });
  });

  el.querySelector('.settings__attach button.attach').addEventListener('click', async () => {
    const dir = await self.showDirectoryPicker({
      id: 'zc',
    });
    await setAttachedDir(dir);
  });
  el.querySelector('.settings__attach button.unattach').addEventListener('click', async () => {
    await setAttachedDir(null);
  });
}

export async function attachDir() {
  await setAttachedDir(await kv.get('attached-dir'));
}

export function renderSettingsPanel() {
  const el = document.querySelector('.settings');

  {
    el.querySelector('.settings__attach').classList.toggle('hidden', !self.showDirectoryPicker);

    const attachEl = el.querySelector('.settings__attach button.attach');
    attachEl.classList.toggle('hidden', attachedDirHandle);

    const unattachEl = el.querySelector('.settings__attach button.unattach');
    unattachEl.classList.toggle('hidden', !attachedDirHandle);
    unattachEl.textContent = `Folder attached: ${attachedDirHandle?.name}–Click to unattach`;
  }

  el.querySelector('.settings__copy').classList.toggle('hidden', !!self.showDirectoryPicker);

  const filesEl = el.querySelector('.settings__browser-files');
  filesEl.innerHTML = '';
  for (const { path, timestamp } of fsReadAllFiles('/local/browser')) {
    const fileEl = createElement('div', 'file');
    fileEl.append(createElement('div', '', path.replace('/local/browser/', '')));
    fileEl.append(createElement('div', '', new Date(timestamp).toLocaleString()));
    fileEl.setAttribute('data-path', path);
    filesEl.append(fileEl);
  }
}
