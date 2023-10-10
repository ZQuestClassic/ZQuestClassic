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
    if (key === 'open') {
      if (value.startsWith('/')) value = value.substr(1);
    }
    url.searchParams.set(key, value);
  }

  // Make friendly.
  url.search = decodeURIComponent(url.search);
  return url.toString();
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

export function formatBytes(bytes1, bytes2, unit) {
  const b = (bytes1 || bytes2);
  let scale = 1;
  let fixedUnits = 0;

  if (unit === undefined) {
    if (b > 1024 ** 2) {
      unit = 'MB';
    } else if (b > 1024) {
      unit = 'KB';
    } else {
      unit = 'B';
    }
  }

  if (unit === 'B') {
    scale = 1;
    fixedUnits = 0;
  } else if (unit === 'KB') {
    scale = 1024;
    fixedUnits = 0;
  } else if (unit === 'MB') {
    scale = 1024 ** 2;
    fixedUnits = 1;
  }

  if (bytes2 === undefined) {
    return `${(bytes1 / scale).toFixed(fixedUnits)} ${unit}`
  }

  return `${(bytes1 / scale).toFixed(fixedUnits)}/${(bytes2 / scale).toFixed(fixedUnits)} ${unit}`
}
