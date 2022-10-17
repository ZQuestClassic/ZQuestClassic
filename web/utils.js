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
