export async function fetchWithProgress(url, opts, progressCb) {
  const response = await fetch(url, opts);

  // Fetch API cannot tell how much of a compressed request's bytes have been downloaded.
  // We only get the response, post-uncompression.
  // But let's still display the progress in terms of transfer size, so it does not seem
  // like more is being downloaded than in reality.
  // This progress bar won't be totally accurate (compression ratios vary for a given response),
  // but it should be good enough.

  const contentLength = Number(response.headers.get('Content-Length'));
  const inflatedContentLength = response.headers.has('Content-Encoding') ?
    Number(response.headers.get('X-Amz-Meta-Inflated-Content-Size')) :
    null;

  let transferSize = contentLength;
  if (!Number.isInteger(contentLength) || !Number.isFinite(contentLength) || contentLength <= 0) {
    transferSize = null;
  }

  const compressionRatio =
    inflatedContentLength && transferSize ? transferSize / inflatedContentLength : 1;

  let responseBytesRecieved = 0;
  return new Response(new ReadableStream({
    async start(controller) {
      const reader = response.body.getReader();
      while (true) {
        const { done, value } = await reader.read();

        if (done) {
          progressCb(responseBytesRecieved * compressionRatio, transferSize, true);
          break;
        }

        responseBytesRecieved += value.byteLength;
        progressCb(responseBytesRecieved * compressionRatio, transferSize, false);
        controller.enqueue(value);
      }
      controller.close();
    },
  }), {
    ok: response.ok,
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

export function fsExists(path) {
  try {
    FS.lookupPath(path);
    return true;
  } catch {
    return false;
  }
}

export function fsReadAllFiles(folder) {
  const files = [];
  if (!fsExists(folder)) return files;

  function impl(curFolder) {
    for (const name of FS.readdir(curFolder)) {
      if (name === '.' || name === '..') continue;

      const path = `${curFolder}/${name}`;
      const { mode, mtime } = FS.lookupPath(path).node;
      if (FS.isFile(mode)) {
        files.push({ path, timestamp: mtime });
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

const allowedHtmlTags = new Set([
  'a', 'b', 'blockquote', 'br', 'code', 'del', 'div', 'em', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
  'hr', 'i', 'li', 'ol', 'p', 'pre', 's', 'span', 'strong', 'sub', 'sup', 'u', 'ul',
]);
// Dropped along with their contents, rather than unwrapped.
const droppedHtmlTags = new Set([
  'button', 'embed', 'form', 'iframe', 'input', 'math', 'noscript', 'object', 'script',
  'select', 'style', 'svg', 'template', 'textarea',
]);

/**
 * Parses untrusted HTML (quest descriptions scraped from other sites) into a fragment
 * that only has basic formatting tags and safe links.
 */
export function sanitizeHtml(html) {
  const doc = new DOMParser().parseFromString(html, 'text/html');

  function clean(parent) {
    for (const node of [...parent.childNodes]) {
      if (node.nodeType === Node.TEXT_NODE) continue;
      if (node.nodeType !== Node.ELEMENT_NODE) {
        node.remove();
        continue;
      }

      const tag = node.localName;
      if (droppedHtmlTags.has(tag)) {
        node.remove();
        continue;
      }

      clean(node);
      if (!allowedHtmlTags.has(tag)) {
        node.replaceWith(...node.childNodes);
        continue;
      }

      const href = tag === 'a' ? node.getAttribute('href') : null;
      for (const attr of [...node.attributes]) {
        if (attr.name !== 'class') node.removeAttribute(attr.name);
      }
      if (href && /^(https?|mailto):/i.test(href.trim())) {
        node.setAttribute('href', href.trim());
        node.setAttribute('target', '_blank');
        node.setAttribute('rel', 'noopener noreferrer nofollow');
      }
    }
  }

  clean(doc.body);
  const fragment = document.createDocumentFragment();
  fragment.append(...doc.body.childNodes);
  return fragment;
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
  const b = (bytes2 || bytes1);
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
