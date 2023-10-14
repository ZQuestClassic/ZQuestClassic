import puppeteer from 'puppeteer';
import statikk from 'statikk';
import { expect } from 'expect';
import { setupConsoleListener } from './utils.js';

/** @type {import('puppeteer').Browser} */
let browser;
/** @type {import('puppeteer').Page} */
let page;
/** @type {ReturnType<typeof statikk>} */
let server;
/** @type {ReturnType<typeof setupConsoleListener>} */
let consoleListener;
let url = '';
let pageErrors = [];
let allMessages = [];

before(async () => {
  browser = await puppeteer.launch({
    headless: !process.env.HEADFULL,
  });
  page = await browser.newPage();
  consoleListener = setupConsoleListener(page);
  page.on('pageerror', pageError => pageErrors.push(pageError));
  page.on('console', e => {
    allMessages.push(e.text());
    if (e.type() === 'error') {
      const error = JSON.stringify({
        text: e.text(),
        stack: e.stackTrace(),
      }, null, 2);
      if (error.includes('favicon.ico')) return;

      // Web build fetches the quest manifest from the internet. In case that ever goes away,
      // don't allow errors in fetching to fail tests.
      if (error.includes('manifest.json') || error.includes('Failed to fetch') || error.includes('blocked by CORS policy')) {
        console.warning(error);
        return;
      }

      // TODO: fix this in native and for web. Maybe causes sound degradation?
      if (error.includes('_al_kcm_refill_stream')) {
        console.warning(error);
        return;
      }

      pageErrors.push(error);
    }
  });
  server = statikk({
    coi: true,
    root: '../build_emscripten/Release/packages/web',
  });
  await new Promise(resolve => server.server.once('listening', resolve));
  url = server.url;
});

after(async () => {
  await browser.close();
  await server.server.close();
});

beforeEach(async () => {
  consoleListener.reset();
  pageErrors = [];
  allMessages = [];
});

async function ensureNoErrors() {
  await page.evaluate(() => new Promise(window.requestAnimationFrame));
  const theErrors = pageErrors;
  pageErrors = [];
  if (theErrors.length) console.log(allMessages);
  expect(theErrors).toHaveLength(0);
}

afterEach(async () => {
  // Tests should call this themselves so the failure is associated with them in the test report,
  // but just in case one is missed it won't hurt to repeat the check here.
  await ensureNoErrors();
});

describe('player', () => {
  it.skip('basic', async () => {
    await page.goto(`${url}/play/?storage=idb`, { waitUntil: 'networkidle0', timeout: 0 });

    await consoleListener.waitFor('Loading Saved Games');
    await new Promise(resolve => setTimeout(resolve, 4000));

    await page.keyboard.press('Enter', { delay: 1000 });

    await page.keyboard.press('ArrowDown', { delay: 1000 });
    await page.keyboard.press('ArrowUp', { delay: 1000 });
    await page.keyboard.press('Enter', { delay: 1000 });

    await page.keyboard.down('h', { delay: 100 });
    await page.keyboard.down('e', { delay: 100 });
    await page.keyboard.down('r', { delay: 100 });
    await page.keyboard.down('o', { delay: 100 });
    await page.keyboard.press('Enter', { delay: 500 });

    await new Promise(resolve => setTimeout(resolve, 10000));
    await page.keyboard.press('k', { delay: 500 });
    await page.keyboard.press('Enter', { delay: 500 });
    await consoleListener.waitFor(`Loading '/_quests/7th/7th.qst'`);
    await new Promise(resolve => setTimeout(resolve, 10000));
  });

  it.skip('basic 2', async () => {
    await page.goto(`${url}/play/?storage=idb&quest=classic/1st.qst`, { waitUntil: 'networkidle0', timeout: 0 });
    if (await page.$('.permission .cancel')) {
      await page.click('.permission .cancel');
    }

    await consoleListener.waitFor('Loading Saved Games');
    await new Promise(resolve => setTimeout(resolve, 4000));

    await page.keyboard.press('Enter', { delay: 500 });

    await new Promise(resolve => setTimeout(resolve, 10000));
  });

  it('loads without errors', async () => {
    await page.goto(`${url}/play/?storage=idb`, { waitUntil: 'networkidle0', timeout: 0 });

    await consoleListener.waitFor('Loading Saved Games');
    await new Promise(resolve => setTimeout(resolve, 4000));

    await page.keyboard.press('Enter', { delay: 1000 });

    await page.keyboard.press('ArrowDown', { delay: 1000 });
    await page.keyboard.press('ArrowUp', { delay: 1000 });
    await page.keyboard.press('Enter', { delay: 1000 });

    await new Promise(resolve => setTimeout(resolve, 5000));
  });

  it.skip('replay classic_1st_lvl1.zplay', async () => {
    await page.goto(`${url}/play/?storage=idb&assert=test_replays/classic_1st_lvl1.zplay&replayExitWhenDone&v0&showFps`, { waitUntil: 'networkidle0', timeout: 0 });
    await consoleListener.waitFor(/exit with code/);
  }).timeout(120_000 * 3);
});

describe('editor', () => {
  it('loads without errors', async () => {
    await page.goto(`${url}/create/?storage=idb`, { waitUntil: 'networkidle0', timeout: 0 });

    await new Promise(resolve => setTimeout(resolve, 10000));
  });
});
