import fs from 'fs';
import * as url from 'url';
import puppeteer from 'puppeteer';
import statikk from 'statikk';
import { setupConsoleListener } from './utils.js';

const buildFolder = process.argv[2];
const target = process.argv[3];
const args = process.argv.slice(4);

const server = statikk({
  coi: true,
  root: buildFolder,
});
await new Promise(resolve => server.server.once('listening', resolve));

const openUrl = new URL(target, server.url);
openUrl.searchParams.append('storage', 'idb');

const camelize = s => s.replace(/-./g, x=>x[1].toUpperCase());

const processedArgs = {};
let previousArg;

for (let arg of args) {
  if (!arg.startsWith('-') && !previousArg) {
    if (processedArgs.open) throw new Error('unexpected arg ' + arg);
    processedArgs.open = arg;
    continue;
  }

  if (!arg.startsWith('-') && previousArg) {
    if (!Array.isArray(processedArgs[previousArg])) {
      processedArgs[previousArg] = [];
    }
    processedArgs[previousArg].push(arg);
    continue;
  }

  arg = camelize(arg);
  arg = arg[0].toLowerCase() + arg.slice(1);
  processedArgs[arg] = true;

  previousArg = arg;
}

if (processedArgs.test) {
  processedArgs.dmap = processedArgs.test[1];
  processedArgs.screen = processedArgs.test[2];
  processedArgs.test = processedArgs.test[0];
}

for (const [key, value] of Object.entries(processedArgs)) {
  if (value === true) {
    openUrl.searchParams.set(key, '');
  } else {
    openUrl.searchParams.set(key, value);
  }
}

async function run() {
  const browser = await puppeteer.launch({
    headless: openUrl.searchParams.has('headless') ? 'new' : false,
  });
  const page = await browser.newPage();

  const consoleListener = setupConsoleListener(page);
  page.on('console', e => {
    const type = e.type();
    if (type === 'error' || type === 'warning') {
      console.error(e.text());
    } else {
      console.log(e.text());
    }
  });

  await page.goto(openUrl, {
    waitUntil: 'networkidle2',
  });

  let hasExited = false;
  let exitCode = -1;
  consoleListener.waitFor(/exit with code:/).then(text => {
    hasExited = true;
    exitCode = Number(text.replace('exit with code:', ''));
  });

  await consoleListener.waitFor(/Initializing Allegro|exit with code/);

  while (!hasExited && !page.isClosed()) {
    await new Promise(resolve => setTimeout(resolve, 1000));
  }

  if (!page.isClosed()) {
    const allegroLog = await page.evaluate(() => {
      return new TextDecoder().decode(FS.readFile('allegro.log'));
    });
    console.log(allegroLog);
  }

  await browser.close();
  await server.server.close();

  return exitCode;
}

process.exit(await run());
