import fs from 'fs';
import path from 'path';
import * as url from 'url';
import puppeteer from 'puppeteer';
import { setupConsoleListener } from './utils.js';

const dirname = url.fileURLToPath(new URL('.', import.meta.url));

const serverUrl = process.argv[2];
const outputFolder = process.argv[3];
const urlPath = process.argv[4];
const extraArgs = process.argv[5] || '';
const resultPath = process.argv[6] || '';

const replayUrl = new URL(urlPath, serverUrl);
const args = extraArgs.split(' ');
let lastKey;
for (let i = 0; i < args.length; i++) {
  if (args[i].startsWith('-')) {
    const key = args[i]
      .substring(1)
      .toLowerCase()
      .split('-')
      .map((it, i) => {
        if (i === 0) return it;
        return it.charAt(0).toUpperCase() + it.substring(1);
      })
      .join('');
    replayUrl.searchParams.set(key, '');
    lastKey = key;
  } else if (lastKey) {
    replayUrl.searchParams.set(lastKey, args[i]);
    lastKey = null;
  }
}

replayUrl.searchParams.append('storage', 'idb');
console.log(replayUrl.href);
const zplay = replayUrl.searchParams.get('assert') || replayUrl.searchParams.get('replay') || replayUrl.searchParams.get('update');
const headless = replayUrl.searchParams.has('headless');
const browser = await puppeteer.launch({
  headless: headless ? 'new' : false,
});

async function runReplay(zplay) {
  const zplaySplit = zplay.split('/');
  const zplayName = zplaySplit[zplaySplit.length - 1];

  const page = await browser.newPage();

  let hasExited = false;
  let exitCode = 0;

  const consoleListener = setupConsoleListener(page);
  page.on('pageerror', e => {
    process.stderr.write(e.toString());
    if (e.stack) process.stderr.write(e.stack);
    process.stderr.write('\n');
    hasExited = true;
  });

  page.on('console', async (e) => {
    const type = e.type();
    const args = await Promise.all(e.args().map(arg => page.evaluate(arg => {
      if (arg instanceof Error)
        return arg.message;
      return arg;
    }, arg).catch((e) => {
      console.error('error in run_replay.js', e);
      return '???';
    })));
    const text = args.join(' ');

    if (type === 'error' || type === 'warning') {
      process.stderr.write(text);
      process.stderr.write('\n');
    } else {
      process.stdout.write(text);
      process.stdout.write('\n');
    }

    const bad = [
      'worker sent an error',
      'ERR_BLOCKED_BY_RESPONSE',
      'ERR_CONNECTION_RESET',
      // shutdown_timers
      // failed: _al_vector_size(&active_timers) == 0, at: /Users/connorclark/code/ZeldaClassic-secondary/build_emscripten/_deps/allegro5-src/src/timernu.c,146,shutdown_timers
      'Uncaught RuntimeError',
      'Aborted(native code called abort())',
      'Assert failed',
    ];
    if (bad.some(t => text.includes(t))) {
      await browser.close();
      process.exit(1);
    }
  });

  await page.goto(replayUrl, {
    waitUntil: 'domcontentloaded',
  });

  consoleListener.waitFor(/exit with code:/).then(text => {
    hasExited = true;
    exitCode = Number(text.replace('exit with code:', ''));
  });

  await consoleListener.waitFor(/Replay is active/);

  async function getResultFile() {
    const result = await page.evaluate((zplay) => {
      if (!FS.findObject(`${zplay}.result.txt`)) {
        return;
      }
      return new TextDecoder().decode(FS.readFile(`${zplay}.result.txt`));
    }, zplay);
    if (!result) return;

    const outPath = resultPath || `${outputFolder}/${zplayName}.result.txt`;
    fs.writeFileSync(outPath, result);
  }
  while (!hasExited) {
    await getResultFile();
    await new Promise(resolve => setTimeout(resolve, 1000));
  }
  await new Promise(resolve => setTimeout(resolve, 1000));
  await getResultFile();

  await page.addScriptTag({ content: fs.readFileSync(`${dirname}/buffer.js`).toString() });
  const zplayDir = path.dirname(zplay);
  const snapshots = await page.evaluate((zplayName, zplayDir) => {
    const files = FS.readdir(zplayDir)
      .filter(file => file.endsWith('.png') && file.includes(zplayName));
    return files.map(file => ({
      file,
      content: Buffer.from(FS.readFile(`${zplayDir}/${file}`)).toString('binary'),
    }));
  }, zplayName, zplayDir);

  for (const { file, content } of snapshots) {
    fs.writeFileSync(`${outputFolder}/${file}`, Buffer.from(content, 'binary'));
  }

  const allegroLog = await page.evaluate(() => {
    return new TextDecoder().decode(FS.readFile('allegro.log'));
  });
  fs.writeFileSync(`${outputFolder}/allegro.log`, allegroLog);

  const roundtrip = await page.evaluate((zplay) => {
    if (FS.findObject(`${zplay}.roundtrip`)) {
      return new TextDecoder().decode(FS.readFile(`${zplay}.roundtrip`));
    }
  }, zplay);
  if (roundtrip) {
    fs.writeFileSync(`${outputFolder}/${zplayName}.roundtrip`, roundtrip);
  }

  return exitCode;
}

let code = 1;
try {
  code = await runReplay(zplay);
} finally {
  await browser.close();
  process.exit(code);
}
