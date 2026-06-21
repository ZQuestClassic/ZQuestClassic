/**
 * @param {import('puppeteer').Page} page
 */
// Cap the retained-message buffer so a page that floods the console (e.g. a
// replay stuck logging an error every frame) can't grow node's heap without
// bound. waitFor() only ever looks for messages emitted early in startup, so
// dropping the oldest entries is safe.
const MAX_SEEN = 5000;

function setupConsoleListener(page) {
  /** @type {string[]} */
  let seen = [];
  page.on('console', (e) => {
    seen.push(e.text());
    if (seen.length > MAX_SEEN * 2) {
      seen.splice(0, seen.length - MAX_SEEN);
    }
  });

  return {
    /**
     * @param {string|RegExp} pattern
     */
    async waitFor(pattern) {
      const matching = seen.find(text => typeof pattern === 'string' ? text === pattern : pattern.test(text));
      if (matching) return matching;
      return waitForConsoleMessage(page, pattern);
    },
    reset() {
      seen = [];
    },
  }
}

/**
 * @param {import('puppeteer').Page} page
 * @param {string|RegExp} pattern
 * @return {Promise<string>}
 */
function waitForConsoleMessage(page, pattern) {
  return new Promise(resolve => {
    page.on('console', function listener(e) {
      const text = e.text();
      if (typeof pattern === 'string' ? text === pattern : pattern.test(text)) {
        page.off('console', listener);
        resolve(text);
      }
    });
  });
}

export {
  setupConsoleListener,
  waitForConsoleMessage,
};
