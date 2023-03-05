/**
 * @param {import('puppeteer').Page} page
 */
async function useLocalStorage(page) {
  const isVisible = await page.evaluate(() => {
    const node = document.querySelector('.permission:not(.hidden) .cancel');
    if (!node)
      return false;
    const style = window.getComputedStyle(node);
    return style && style.display !== 'none' && style.visibility !== 'hidden' && style.opacity !== '0';
  });
  if (isVisible) {
    await page.click('.permission .cancel');
  }
}

/**
 * @param {import('puppeteer').Page} page
 */
function setupConsoleListener(page) {
  /** @type {string[]} */
  let seen = [];
  page.on('console', (e) => {
    seen.push(e.text());
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
  useLocalStorage,
  setupConsoleListener,
  waitForConsoleMessage,
};
