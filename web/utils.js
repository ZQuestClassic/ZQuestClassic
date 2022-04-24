export async function fetchWithProgress(url, opts, progressCb) {
  const response = await fetch(url, opts);

  let received = 0;
  let contentLength = Number(response.headers.get('Content-Length')) || 0;
  if (!Number.isInteger(contentLength) || !Number.isFinite(contentLength) || contentLength <= 0) {
    contentLength = null;
  }

  return new Response(new ReadableStream({
    async start(controller) {
      const reader = response.body.getReader();
      while (true) {
        const { done, value } = await reader.read();

        if (done) {
          progressCb(received, contentLength, true);
          break;
        }

        received += value.byteLength;
        progressCb(received, contentLength, false);
        controller.enqueue(value);
      }
      controller.close();
    },
  }), {
    status: response.status,
    statusText: response.statusText,
    headers: response.headers,
  });
}

export function createUrlString(pathname, params) {
  const url = new URL(pathname, location.href);
  url.search = '';

  for (let [key, value] of Object.entries(params)) {
    if (key === 'quest') {
      if (value.startsWith('/_quests/')) value = value.replace('/_quests/', '');
      if (value.startsWith('/')) value = value.substr(1);
    }
    url.searchParams.set(key, value);
  }

  return url.toString().replace(/%2F/g, '/');
}

export function fsReadAllFiles(folder) {
  const files = [];
  if (!FS.analyzePath(folder).exists) return files;

  function impl(curFolder) {
    for (const name of FS.readdir(curFolder)) {
      if (name === '.' || name === '..') continue;

      const path = `${curFolder}/${name}`;
      const { mode, timestamp } = FS.lookupPath(path).node;
      if (FS.isFile(mode)) {
        files.push({ path, timestamp });
      } else if (FS.isDir(mode)) {
        impl(path);
      }
    }
  }

  impl(folder);
  return files;
}

export function createElement(tagName, className, textContent) {
  const el = document.createElement(tagName);
  if (className) el.className = className;
  el.textContent = textContent;
  return el;
}

export const getFsHandles = async (dirHandle) => {
  const handles = [];

  async function collect(curDirHandle) {
    for await (const entry of curDirHandle.values()) {
      handles.push(entry);
      if (entry.kind === 'directory') {
        await collect(entry);
      }
    }
  }

  await collect(dirHandle);

  const result = new Map();
  result.set('.', dirHandle);
  for (const handle of handles) {
    const relativePath = (await dirHandle.resolve(handle)).join('/');
    result.set(relativePath, handle);
  }
  return result;
};
