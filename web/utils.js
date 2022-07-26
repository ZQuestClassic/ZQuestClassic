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

export function mkdirp(folderPath) {
  const pathParts = folderPath.split('/')
  let dirPath = '/'
  for (let i = 0; i < pathParts.length; i++) {
    const curPart = pathParts[i]
    try {
      FS.mkdir(`${dirPath}${curPart}`)
    } catch (err) { }
    dirPath += `${curPart}/`
  }
};

export function ensureFolderExists(path) {
  const folderPath = path
    .split('/')
    .slice(0, -1) // remove basename
    .join('/');
  mkdirp(folderPath);
};

export function createElement(tagName, className, textContent) {
  const el = document.createElement(tagName);
  if (className) el.className = className;
  el.textContent = textContent;
  return el;
}
