import { fileSave } from "browser-fs-access";
import * as kv from "idb-keyval";
import { createElement, fsReadAllFiles, mkdirp } from "./utils.js";

/** @type {FileSystemDirectoryHandle} */
let attachedDirHandle = null;

async function setAttachedDir(dir) {
  const options = { mode: 'readwrite' };
  if (dir && await dir.queryPermission(options) !== 'granted') {
    if (await dir.requestPermission(options) !== 'granted') {
      dir = null;
    }
  }

  if (attachedDirHandle) FS.unmount('/local/filesystem');
  attachedDirHandle = dir;
  if (dir) {
    await kv.set('attached-dir', dir);
  } else {
    await kv.del('attached-dir');
  }

  renderSettingsPanel();

  if (!dir) return;

  await import('./fsfs.js');
  mkdirp('/local/filesystem');
  FS.mount(FSFS, { dirHandle: attachedDirHandle }, '/local/filesystem');
  await new Promise((resolve, reject) => FS.syncfs(true, (err) => {
    if (err) return reject(err);
    resolve();
  }));
}

export async function setupSettingsPanel() {
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
    setAttachedDir(dir);
  });
  el.querySelector('.settings__attach button.unattach').addEventListener('click', async () => {
    setAttachedDir(null);
  });

  // TODO: work out requesting for permission prompt.
  // setAttachedDir(await kv.get('attached-dir'));
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
