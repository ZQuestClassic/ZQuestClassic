import fs from 'fs';
import path from 'path';
import os from 'os';
import * as url from 'url';
import puppeteer from 'puppeteer';
import statikk from 'statikk';
import { setupConsoleListener } from './utils.js';

const dirname = url.fileURLToPath(new URL('.', import.meta.url));

const buildFolder = process.argv[2];
const outputFolder = process.argv[3];
const urlPath = process.argv[4];

const server = statikk({
  coi: true,
  root: buildFolder,
});
await new Promise(resolve => server.server.once('listening', resolve));

const replayUrl = new URL(urlPath, server.url);
replayUrl.searchParams.append('storage', 'idb');
const zplay = replayUrl.searchParams.get('assert') || replayUrl.searchParams.get('replay');

async function runReplay(zplay) {
  const zplaySplit = zplay.split('/');
  const zplayName = zplaySplit[zplaySplit.length - 1];

  const browser = await puppeteer.launch({
    headless: !process.env.HEADFULL,
  });
  const page = await browser.newPage();

  const stdoutFd = fs.openSync(`${outputFolder}/stdout.txt`, 'w');
  const stderrFd = fs.openSync(`${outputFolder}/stderr.txt`, 'w');

  const consoleListener = setupConsoleListener(page);
  page.on('console', e => {
    const type = e.type();
    if (type === 'error' || type === 'warning') {
      fs.writeSync(stderrFd, e.text());
      fs.writeSync(stderrFd, '\n');
    } else {
      fs.writeSync(stdoutFd, e.text());
      fs.writeSync(stdoutFd, '\n');
    }
  });

  await page.goto(replayUrl, {
    waitUntil: 'networkidle0',
  });

  let hasExited = false;
  let exitCode = 0;
  consoleListener.waitFor(/exit with code:/).then(text => {
    hasExited = true;
    exitCode = Number(text.replace('exit with code:', ''));
  });

  await new Promise(resolve => setTimeout(resolve, 5000));

  const zplayExists = await page.evaluate((zplay) => {
    return !!FS.findObject(zplay);
  }, zplay);
  if (!zplayExists) {
    throw new Error(`could not find replay: ${zplay}`);
  }

  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'zc-replays-'));
  async function getResultFile() {
    const result = await page.evaluate((zplay) => {
      if (!FS.findObject(`${zplay}.result.txt`)) {
        return;
      }
      return new TextDecoder().decode(FS.readFile(`${zplay}.result.txt`));
    }, zplay);
    if (!result) return;

    const tmpPath = `${tmpDir}/tmp.result.txt`;
    fs.writeFileSync(tmpPath, result);
    fs.renameSync(tmpPath, `${outputFolder}/${zplayName}.result.txt`);
  }
  while (!hasExited) {
    await getResultFile();
    await new Promise(resolve => setTimeout(resolve, 1000));
  }
  await getResultFile();

  await page.addScriptTag({ content: fs.readFileSync(`${dirname}/buffer.js`).toString() });
  const snapshots = await page.evaluate((zplayName) => {
    const files = FS.readdir('/test_replays')
      .filter(file => file.endsWith('.png') && file.includes(zplayName));
    return files.map(file => ({
      file,
      content: Buffer.from(FS.readFile(`/test_replays/${file}`)).toString('binary'),
    }));
  }, zplayName);

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

  fs.closeSync(stdoutFd);
  fs.closeSync(stderrFd);

  await browser.close();
  await server.server.close();
  return exitCode;
}

process.exit(await runReplay(zplay));
