// npm run collect
// downloads all pzc quests, external music, and metadata to .tmp/database

import fs from 'fs';
import path from 'path';
import {fileURLToPath} from 'url';
import {execFileSync} from 'child_process';
import crypto from 'crypto';
import {Readable} from 'stream';
import {finished} from 'stream/promises';
import decompress from 'decompress';
import * as glob from 'glob';
import * as puppeteer from 'puppeteer';

const DIR = path.dirname(fileURLToPath(new URL(import.meta.url)));
const ROOT = path.join(DIR, '../..');
const TMP = path.join(ROOT, '.tmp');
const DB = path.join(TMP, 'database');
const ARCHIVES = path.join(TMP, 'database_archives');
fs.mkdirSync(DB, {recursive: true});
fs.mkdirSync(ARCHIVES, {recursive: true});

interface QuestManifest {
  id: string;
  index: number;
  name: string;
  projectUrl: string;
  dateAdded: string;
  dateUpdated?: string;
  author: string;
  contentHash: string;
  resources: string[];
  videoUrl?: string;
  images: string[];
  genre: string;
  zcVersion: string;
  informationHtml?: string;
  descriptionHtml: string;
  storyHtml: string;
  tipsAndCheatsHtml: string;
  creditsHtml: string;
  rating: {
    score: number;
    distribution: number[];
  };
}

const outputFile = `${DB}/manifest.json`;
const doNotExistFile = `${TMP}/quest_db_donotexist.json`;

let questsMap: Map<string, QuestManifest>;
let doNotExist: string[] = [];

function loadQuests() {
  questsMap = new Map<string, QuestManifest>();
  if (fs.existsSync(outputFile)) {
    const quests: Record<string, QuestManifest> = JSON.parse(fs.readFileSync(outputFile, 'utf-8'));
    for (const quest of Object.values(quests)) {
      questsMap.set(quest.id, quest);
    }
  }

  doNotExist = [];
  if (fs.existsSync(doNotExistFile)) {
    doNotExist = JSON.parse(fs.readFileSync(doNotExistFile, 'utf-8'));
  }
}

function getFirstQstFile(quest: QuestManifest) {
  const qst = quest.resources.find(r => path.extname(r).toLowerCase() === '.qst');
  if (!qst) {
    throw new Error('found no qst file');
  }
  return qst;
}

function saveQuests() {
  const entries = [...questsMap.entries()]
    .sort((a, b) => a[1].index - b[1].index);
  const obj = Object.fromEntries(entries);
  fs.writeFileSync(outputFile, JSON.stringify(obj, null, 2));
  fs.writeFileSync(doNotExistFile, JSON.stringify(doNotExist, null, 2));
}

function download(url: string, outputPath: string) {
  return new Promise(async (resolve, reject) => {
      const res = await fetch(url);
      if (!res.body) throw new Error(`body is null: ${url}`);

      const fileStream = fs.createWriteStream(outputPath, { flags: 'wx' });
      finished(Readable.fromWeb(res.body).pipe(fileStream));
      fileStream.on('error', reject);
      fileStream.on('finish', resolve);
    });
}

function getFileHash(path: string): Promise<string> {
  return new Promise(resolve => {
    const fd = fs.createReadStream(path);
    const hash = crypto.createHash('sha1');
    hash.setEncoding('hex');
    fd.on('end', function() {
      hash.end();
      resolve(hash.read());
    });
    fd.pipe(hash);
  });
}

async function downloadGoogleDriveUrl(page: puppeteer.Page, url: string, destination: string) {
  const id = new URL(url).searchParams.get('id') || (url.match(/([^\/]{20,})/) || [])[0];
  const exportUrl = `https://drive.google.com/uc?export=download&id=${id}`;
  const maybeDirectUrl = execFileSync('curl', ['-Ls', '-o', '/dev/null', '-w', '%{url_effective}', exportUrl], {encoding: 'utf-8'});
  if (maybeDirectUrl.includes('googleusercontent.com')) {
    await download(maybeDirectUrl, destination);
    return;
  }

  const client = await page.target().createCDPSession()
  fs.rmSync(`${ARCHIVES}/drive`, {force: true, recursive: true});
  await client.send('Page.setDownloadBehavior', {
    behavior: 'allow',
    downloadPath: `${ARCHIVES}/drive`,
  });
  
  await page.goto(exportUrl);
  await page.evaluate(() => {
    // @ts-expect-error
    return document.querySelector('#download-form').submit();
  });

  await waitUntilDownload(page);
  await client.detach();

  fs.copyFileSync(glob.sync(`${ARCHIVES}/drive/*`)[0], destination);
  fs.rmSync(`${ARCHIVES}/drive`, {force: true, recursive: true});
}

async function fetchExternalMusic(page: puppeteer.Page, quest: QuestManifest) {
  const text = quest.descriptionHtml + (quest.informationHtml || '');
  const match = text.match(/(https?:\/\/([^\s]*?)\.zip)/) || text.match(/(https?:\/\/drive\.google\.com([^\s]*))"/);
  if (!match) return;

  let url = match[1];

  if (url === 'http://www.shardstorm.com/zcmirror/zc-250-win.zip') {
    return;
  }

  if (url === 'https://www.dropbox.com/s/31kdsjdd1n8mxt4/TLoZ%20Beginnings%20Music.zip') {
    // 404
    return;
  }

  // Some people link directly to the mediafire zip file... and those links tend to die.
  url = {
    'http://download1855.mediafire.com/yfbcefc4vulg/641lva6riprra7j/Music.zip': 'http://www.mediafire.com/file/641lva6riprra7j/Music.zip',
    'http://download1688.mediafire.com/5nccux8bvp3g/2llbdzzq8heiuqk/Mayath+Music.zip': 'http://www.mediafire.com/file/2llbdzzq8heiuqk/Mayath+Music.zip',
  }[url] || url;

  const destZip = `${ARCHIVES}/music/${quest.id}/${path.basename(url)}`;
  if (!fs.existsSync(destZip)) {
    fs.mkdirSync(path.dirname(destZip), {recursive: true});
    const host = new URL(url).host;
    const directAccessHosts = [
      'cdn.discordapp.com',
      'bloodstar.rustedlogic.net',
    ];
  
    let handled = false;
    if (host === 'www.mediafire.com') {
      await page.goto(url);
      url = await page.evaluate(() => {
        // @ts-expect-error
        return document.querySelector('*[aria-label="Download file"]')?.href;
      });
      if (!url) throw new Error('...');
    } else if (host === 'drive.google.com' && (url.includes('/open') || url.includes('/file'))) {
      handled = true;
      console.log(`[${quest.id}] downloading ${url}`);
      await downloadGoogleDriveUrl(page, url, destZip);
    } else if (directAccessHosts.includes(host)) {
      // OK!
    } else if (host === 'www.dropbox.com') {
      url += '?dl=1'
    } else {
      throw new Error(`unknown host provider: ${url}`);
    }
  
    if (!handled) {
      console.log(`[${quest.id}] downloading ${url}`);
      await download(url, destZip);
    }
  }

  const questDir = `${DB}/${quest.id}`;
  console.log(`[${quest.id}] unzipping ${destZip}`);
  await decompress(destZip, questDir, {strip: 1});
}

async function getLatestPzcId(page: puppeteer.Page) {
  await page.goto('https://www.purezc.net/index.php?page=quests&sort=added', { waitUntil: 'networkidle2' });
  return await page.evaluate(() => {
    // @ts-expect-error
    return Number(new URL(document.querySelector('tbody td a').href).searchParams.get('id'));
  });
}

async function processId(page: puppeteer.Page, pzcId: number) {
  if ([81, 178, 400, 401].includes(pzcId)) {
    // Zip is busted - or maybe I'm using a bad library.
    return;
  }

  const id = `quests/purezc/${pzcId}`;
  const questDir = `${DB}/${id}`;

  const allImgResponses: Record<string, puppeteer.HTTPResponse> = {};
  page.on('response', (response) => {
    if (response.request().resourceType() === 'image') {
      allImgResponses[response.url()] = response;
    }
  });

  const projectUrl = `https://www.purezc.net/index.php?page=quests&id=${pzcId}`;
  const response = await page.goto(projectUrl, { waitUntil: 'networkidle2' });
  if (!response || response.status() !== 200) {
    console.log(`[${id}] bad response`);
    doNotExist.push(id);
    return;
  }

  const nameRaw = (await page.title()).split('-')[0];
  const metadataRaw1 = await page.evaluate(() => {
    return [...document.querySelectorAll('.ipsBox_container span')].map((e) => e.textContent || '');
  });
  const html = await page.evaluate(() => {
    function clean(el: Element) {
      for (const childEl of el.querySelectorAll('*')) {
        childEl.removeAttribute('style');

        if (childEl.classList.contains('bbc_emoticon') || childEl.classList.contains('bbc_img')) {
          // https://www.purezc.net/forums/public/style_emoticons/default/icon_smile.gif
          let newText = childEl.getAttribute('alt');
          if (!newText) {
            newText = {
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_smile.gif': ':)',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_frown.gif': ':(',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_wink.gif': ';)',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_biggrin.gif': ':D',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_sweat.gif': ':/',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_sorry.gif': ':/',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_lol.gif': ':p',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_razz.gif': ':p',
              'https://www.purezc.net/forums/public/style_emoticons/default/icon_thumbsup.gif': '👍',
            }[childEl.getAttribute('src') || ''] || '';
          }
          childEl.replaceWith(newText || '');
        }
      }
    }

    function collect(el: Element) {
      clean(el);
      return el.innerHTML;
    }

    return {
      tableRows: [...document.querySelectorAll('#item_contentBox .table_row')].map(collect),
      // @ts-expect-error
      entryInfo: collect(document.querySelector('.entryInfo')),
    };
  });

  const imagesRaw = await page.evaluate(() => {
    return [...document.querySelectorAll('#imagelist2 img')]
      .map((e: any) => e.src)
      .filter(url => !url.includes('youtube'));
  });

  const videoUrl = await page.evaluate(() => {
    const el = document.querySelector('#videoPreviewBox iframe');
    if (!el) return;

    // @ts-expect-error
    return el.src;
  });

  const rating = await page.evaluate(() => {
    const el = document.querySelector('*[data-rating]');
    if (!el) {
      return {
        score: 0,
        distribution: [],
      };
    }

    return {
      score: Number(el.getAttribute('data-rating')),
      distribution: eval(el.getAttribute('data-distribution') || '').map(Number),
    };
  });

  const trim = (str: string) => str.replace(/\s+/g, ' ').trim();

  let name = trim(nameRaw);
  if (name[0] === '"' && name[name.length - 1] === '"') name = name.substring(1, name.length - 1);

  const findMetadata = (pattern: RegExp) => {
    for (const raw of metadataRaw1) {
      const m = raw.match(pattern);
      if (m) return trim(m[1]);
    }
  };

  const author = findMetadata(/Creator: (.*)/ms);
  const dateAdded = findMetadata(/Added: (.*)/ms);
  const dateUpdated = findMetadata(/Updated: (.*)/ms);
  const genre = findMetadata(/Genre: (.*)/ms)
  const zcVersion = findMetadata(/ZC Version: (.*)/ms);
  if (!dateAdded || !author || !genre || !zcVersion) {
    console.log({dateAdded, author, genre, zcVersion});
    throw new Error();
  }

  const isNew = !questsMap.get(id);
  const hasMetadataUpdated = !isNew && questsMap.get(id)?.dateUpdated !== dateUpdated;
  if (!isNew && !hasMetadataUpdated && !process.env['FORCE_UPDATE']) {
    console.log(`[${id}] nothing to update`);
    return;
  }

  const archivePath = `${ARCHIVES}/quests/purezc/${pzcId}.zip`;
  fs.mkdirSync(`${ARCHIVES}/quests/purezc`, {recursive: true});
  fs.rmSync(archivePath, {force: true});
  await download(`https://www.purezc.net/index.php?page=download&section=Quests&id=${pzcId}`, archivePath);
  const contentHash = await getFileHash(archivePath);

  const contentHasUpdated = !isNew && contentHash !== questsMap.get(id)?.contentHash;
  if (contentHasUpdated) {
    console.log(`[${id}] content has updated`);
    // TODO
  } else if (isNew) {
    console.log(`[${id}] downloading new quest`);
  } else {
    console.log(`[${id}] metadata has updated`);
  }

  if (contentHasUpdated || isNew) {
    fs.rmSync(questDir, {force: true, recursive: true});
    fs.mkdirSync(questDir, {recursive: true});
    await decompress(archivePath, questDir, {map(f) {
      // console.log(path.dirname(f.path), path.join(questDir, path.dirname(f.path)));
      // fs.mkdirSync(path.join(questDir, path.dirname(f.path)), {recursive: true});
      // console.log(f);
      return f;
    }});
  }

  let informationHtml: string|undefined = trim(html.entryInfo);
  const descriptionHtml = trim(html.tableRows[1]);
  const storyHtml = trim(html.tableRows[3]);
  const tipsAndCheatsHtml = trim(html.tableRows[5]);
  const creditsHtml = trim(html.tableRows[7]);

  if (informationHtml.includes('View Full Description') || informationHtml === descriptionHtml) {
    informationHtml = undefined;
  }

  const images: string[] = [];
  for (let i = 0; i < imagesRaw.length; i++) {
    const resp = allImgResponses[imagesRaw[i]];
    if (!resp) continue;

    const ext = imagesRaw[i].match(/\.(\w+)$/)[1];
    const imageUrl = `${questDir}/image${i}.${ext}`;
    fs.writeFileSync(`${imageUrl}`, await resp.buffer());
    images.push(`image${i}.${ext}`);
  }

  // Fix file names like:
  //  435/Eddy&#39;s Troll Day.qst
  //  782/Mysteries of the Cup & Puzzles of Hyrule & First GameBoy/Puzzles of Hyrule.qst
  {
    let quit = false;
    while (!quit) {
      quit = true;
      for (let file of glob.sync('**/*.qst', { nocase: false, cwd: questDir })) {
        const parts = file.split('/');
        const partsSanitized = parts.map(p => p.replace(/&#39;|[#&;]/g, '').replace(/\s+/g, ' '));
        const firstBadPartIndex = partsSanitized.findIndex((p, i) => p !== parts[i]);
        if (firstBadPartIndex === -1) continue;

        const i = firstBadPartIndex;
        const sanitized = partsSanitized[i];
        const from = parts.slice(0, i + 1).join('/');
        const to = parts.slice(0, i + 1).map((p, j) => i === j ? sanitized : p).join('/');
        fs.renameSync(`${questDir}/${from}`, `${questDir}/${to}`);

        // Must rescan directory.
        quit = false;
        break;
      }
    }
  }

  const quest: QuestManifest = {
    id,
    index: pzcId,
    name,
    projectUrl,
    dateAdded,
    dateUpdated,
    author,
    contentHash,
    resources: [],
    videoUrl,
    images,
    genre,
    zcVersion,
    informationHtml,
    descriptionHtml,
    storyHtml,
    tipsAndCheatsHtml,
    creditsHtml,
    rating,
  };

  await fetchExternalMusic(page, quest);
  quest.resources = glob.sync('**/*', {cwd: questDir, nodir: true}).filter(r => {
    return !images.includes(r);
  }).sort();

  getFirstQstFile(quest);

  questsMap.set(quest.id, quest);
}

async function waitUntilDownload(page: puppeteer.Page) {
  return new Promise<void>((resolve, reject) => {
      page._client().on('Page.downloadProgress', e => { // or 'Browser.downloadProgress'
          if (e.state === 'completed') {
              resolve();
          } else if (e.state === 'canceled') {
              reject();
          }
      });
  });
}

async function main() {
  loadQuests();

  const browser = await puppeteer.launch({headless: 'new'});
  const page = await browser.newPage();

  if (process.env['ONE_SHOT']) {
    await processId(page, Number(process.env['ONE_SHOT']));
  } else {
    const start = Number(process.env['START']) || 1;
    const max = Number(process.env['MAX']) || await getLatestPzcId(page);
    console.log(`processing ${max - start + 1} quests`);
    for (let i = start; i <= max; i++) {
      const id = `quests/purezc/${i}`;
      if (process.env['ONLY_NEW'] && questsMap.has(id)) {
        continue;
      }
      if (doNotExist.includes(id)) {
        continue;
      }
  
      try {
        console.log(`[${id}]`);
        await processId(page, i);
      } catch (e) {
        console.error(id, e);
      }
  
      if (i % 10 === 0) saveQuests();
    }
  }

  console.log('\nexiting');
  saveQuests();
  await browser.close();
}

main();
