// npm run collect
// downloads all pzc quests, external music, and metadata to .tmp/database

import fs from 'fs';
import path from 'path';
import {fileURLToPath} from 'url';
import {execFileSync} from 'child_process';
import crypto from 'crypto';
import * as zlib from 'zlib';
import {Readable} from 'stream';
import {finished} from 'stream/promises';
import decompress from 'decompress';
import * as glob from 'glob';
import * as puppeteer from 'puppeteer';

const OFFICIAL = Boolean(process.env.OFFICIAL);
const OFFICIAL_SYNC = Boolean(process.env.OFFICIAL_SYNC);
const ONLY_NEW = Boolean(process.env.ONLY_NEW);
const ONE_SHOT = Number(process.env.ONE_SHOT);
const START = Number(process.env.START);
const MAX = Number(process.env.MAX);
const FORCE = Boolean(process.env.FORCE_UPDATE);
const PZC_UN = process.env.PZC_UN;
const PZC_PW = process.env.PZC_PW;

const DIR = path.dirname(fileURLToPath(new URL(import.meta.url)));
const ROOT = path.join(DIR, '../..');
const TMP = path.join(ROOT, '.tmp');
const DB = path.join(TMP, 'database');
const ARCHIVES = path.join(TMP, 'database_archives');
fs.mkdirSync(DB, {recursive: true});
fs.mkdirSync(ARCHIVES, {recursive: true});

interface Author {
  id?: number;
  name: string;
  lastLogin?: Date;
  sentApprovalMessage?: boolean;
}

interface QuestManifest {
  id: string;
  index: number;
  name: string;
  projectUrl: string;
  dateAdded: string;
  dateUpdated?: string;
  /**
   * auto: approved on basis of inactive author
   * 
   * pending: waiting for response to approval request
   * 
   * disallowed: not allowed on basis of too similar to Zelda-1, or similar reason
   */
  approval: 'auto' | 'pending' | 'disallowed' | boolean;
  authors: Array<{name: string, id?: number}>;
  /** First is most recent. */
  releases: Array<{
    name: string;
    date: string;
    /** Hash of the zip downloaded from PZC. Does not include external music. */
    hash: string;
    resources: string[];
  }>;
  defaultPath: string;
  /** External music. */
  music: string[];
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
const doNotExistFile = `${DB}/quest_db_donotexist.json`;

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

function getFirstQstFile(release: QuestManifest['releases'][number]) {
  const qst = release.resources.find(r => path.extname(r).toLowerCase() === '.qst');
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

let authorsMap = new Map<number, Author>();
function loadAuthors() {
  authorsMap = new Map();
  const file = `${DB}/authors.json`;
  if (fs.existsSync(file)) {
    const entries: Record<number, Author> = JSON.parse(fs.readFileSync(file, 'utf-8'));
    for (const [key, entry] of Object.entries(entries)) {
      const id = Number(key);
      if (entry.lastLogin) entry.lastLogin = new Date(entry.lastLogin);
      authorsMap.set(id, entry);
    }
  }
}

function saveAuthors() {
  const file = `${DB}/authors.json`;
  const entries = [...authorsMap.entries()];
  const obj = Object.fromEntries(entries);
  fs.writeFileSync(file, JSON.stringify(obj, null, 2));
}

let cache: any;
function loadCache() {
  cache = {};
  const file = `${DB}/cache.json`;
  if (fs.existsSync(file)) {
    cache = JSON.parse(fs.readFileSync(file, 'utf-8'));
  }

  cache.sizeHeaders = cache.sizeHeaders || {};
  cache.encodingHeaders = cache.encodingHeaders || {};
}

function saveCache() {
  const file = `${DB}/cache.json`;
  fs.writeFileSync(file, JSON.stringify(cache, null, 2));
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

async function fetchExternalMusic(page: puppeteer.Page, quest: QuestManifest, resourcesDir: string) {
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

  console.log(`[${quest.id}] unzipping ${destZip}`);
  await decompress(destZip, resourcesDir, {strip: 1});
}

async function getLatestPzcId(page: puppeteer.Page) {
  await page.goto('https://www.purezc.net/index.php?page=quests&sort=added', { waitUntil: 'networkidle2' });
  return await page.evaluate(() => {
    // @ts-expect-error
    return Number(new URL(document.querySelector('tbody td a').href).searchParams.get('id'));
  });
}

async function uncompressQstAndGzip(qstPath: string) {
  const gzPath = qstPath + '.gz';
  if (fs.existsSync(gzPath)) {
    // Already done.
    return;
  }

  // First release with -uncompress-qst
  const releasePath = `${TMP}/releases/nightly-2023-10-11-2`;
  if (!fs.existsSync(releasePath)) {
    console.error(`not found: ${releasePath}`);
    process.exit(1);
  }

  const zeditorPath = releasePath + '/' + glob.sync('**/zeditor', {cwd: releasePath})[0];
  if (!fs.existsSync(zeditorPath)) {
    console.error(`not found: ${zeditorPath}`);
    process.exit(1);
  }

  const out = `${TMP}/out.qst`;
  if (fs.existsSync(out)) fs.unlinkSync(out);
  execFileSync(zeditorPath, [
    '-uncompress-qst',
    qstPath,
    out,
  ], {stdio: 'inherit', cwd: path.dirname(zeditorPath)});

  await new Promise((resolve, reject) => {
    fs.createReadStream(out)
      .pipe(zlib.createGzip())
      .pipe(fs.createWriteStream(gzPath))
      .on('error', reject)
      .on('finish', resolve);
  });
  if (fs.existsSync(out)) fs.unlinkSync(out);
}

async function processId(page: puppeteer.Page, pzcId: number) {
  if ([81, 178, 400, 401].includes(pzcId)) {
    // Zip is busted - or maybe I'm using a bad library.
    return;
  }
  if ([73, 793].includes(pzcId)) {
    // qst doesn't open
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
  const authors = await page.evaluate(() => {
    const els = document.querySelectorAll('.ipsBox_container .table_row .name');
    return [...els].map(el => ({name: el.textContent, id: Number(el.getAttribute('hovercard-id'))}));
  }) as QuestManifest['authors'];
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

  const authorRaw = findMetadata(/Creator: (.*)/ms) || '';
  const dateAdded = findMetadata(/Added: (.*)/ms);
  const dateUpdated = findMetadata(/Updated: (.*)/ms);
  const genre = findMetadata(/Genre: (.*)/ms)
  const zcVersion = findMetadata(/ZC Version: (.*)/ms);

  for (const author of authorRaw.split(',').map(a => a.trim())) {
    if (authors.some(a => a.name === author)) continue;

    authors.push({name: author});
  }

  if (!dateAdded || !authors.length || !genre || !zcVersion) {
    console.log({dateAdded, authors, genre, zcVersion});
    throw new Error();
  }

  const existingManifestEntry = questsMap.get(id);
  const isNew = !existingManifestEntry;
  const hasMetadataUpdated = !isNew && existingManifestEntry?.dateUpdated !== dateUpdated;
  if (!isNew && !hasMetadataUpdated && !FORCE) {
    console.log(`[${id}] nothing to update`);
    return;
  }

  if (hasMetadataUpdated) {
    console.log(`[${id}] previously updated at: ${existingManifestEntry.dateUpdated}. Found new update from: ${dateUpdated}`);
  }

  const archivePath = `${ARCHIVES}/quests/purezc/${pzcId}.zip`;
  fs.mkdirSync(`${ARCHIVES}/quests/purezc`, {recursive: true});
  // TODO: if this download has etag or last modified or w/e, could avoid re-downloaded when nothing changed.
  if (!FORCE) {
    fs.rmSync(archivePath, {force: true});
    await download(`https://www.purezc.net/index.php?page=download&section=Quests&id=${pzcId}`, archivePath);
  }
  const contentHash = await getFileHash(archivePath);

  let thisRelease;
  const releases = existingManifestEntry?.releases || [];
  const mostRecentRelease = existingManifestEntry?.releases[0];
  const contentHashUpdated = mostRecentRelease && contentHash !== mostRecentRelease?.hash;

  if (isNew || contentHashUpdated) {
    if (contentHashUpdated) console.log(`[${id}] content has updated`);
    else console.log(`[${id}] downloading new quest`);

    const r = releases.length + 1;
    thisRelease = {
      name: `r${r.toString().padStart(2, '0')}`,
      date: dateUpdated || dateAdded || '',
      hash: contentHash,
      resources: [],
    };
    releases.unshift(thisRelease);
  } else {
    if (!mostRecentRelease) throw new Error('unexpected');

    console.log(`[${id}] metadata has updated`);
    thisRelease = mostRecentRelease;
  }

  let hasUnzippedResources = false;
  const resourcesDir = `${questDir}/${thisRelease.name}`;
  fs.mkdirSync(resourcesDir, {recursive: true});
  if (isNew || contentHashUpdated || FORCE) {
    await decompress(archivePath, resourcesDir, {map(f) {
      return f;
    }});
    hasUnzippedResources = true;
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
  if (hasUnzippedResources) {
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

    thisRelease.resources = glob.sync('**/*', {cwd: resourcesDir, nodir: true}).sort().filter(r => !r.endsWith('.gz'));
    getFirstQstFile(thisRelease);
    for (const qst of thisRelease.resources.filter(r => r.endsWith('.qst'))) {
      const path = `${DB}/${id}/${thisRelease.name}/${qst}`;
      // Uncompress the contents of the qst file, so that loading code is a bit faster.
      // Should still compress for the network, so let's gzip it. Gzip is actually way better than a4 PACKFILE (Yuurand.qst was 43M -> 27M).
      await uncompressQstAndGzip(path);
    }
  }

  const qsts = thisRelease.resources.filter(r => r.endsWith('.qst'));
  const qst = qsts[0];
  // TODO: Don't forget previous value.
  const defaultPath = `${id}/${thisRelease.name}/${qst}`;

  const quest: QuestManifest = {
    id,
    index: pzcId,
    name,
    projectUrl,
    dateAdded,
    dateUpdated,
    approval: existingManifestEntry?.approval ?? 'pending',
    authors,
    releases,
    defaultPath,
    music: existingManifestEntry?.music ?? [],
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

  const musicDir = `${questDir}/music`;
  try {
    await fetchExternalMusic(page, quest, musicDir);
    quest.music = glob.sync('**/*', {cwd: musicDir, nodir: true}).sort();
  } catch (e) {
    console.log(`[${id}] error fetching external music`);
    console.error(`[${id}]`, e);
  }

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

function groupByAuthor() {
  // For object equality goodness.
  const authorCache: Record<string, Author> = {};
  function getAuthor(name: string, id: number|undefined) {
    if (id) {
      const author = authorsMap.get(id);
      if (!author) throw new Error(`unknown author: ${name} ${id}`);
      return author;
    }
    if (authorCache[name]) return authorCache[name];
    return authorCache[name] = {name};
  }

  const questsByAuthor = new Map<Author, QuestManifest[]>();
  for (const quest of questsMap.values()) {
    for (const rawAuthor of quest.authors) {
      const author = getAuthor(rawAuthor.name, rawAuthor.id);
      const quests = questsByAuthor.get(author) || [];
      questsByAuthor.set(author, quests);
      quests.push(quest);
    }
  }
  return new Map([...questsByAuthor].sort((a, b) => b[1].length - a[1].length));
}

async function forEveryQst(cb: (args: {quest: QuestManifest, release: QuestManifest['releases'][0], qst: string, path: string}) => Promise<void>|void) {
  for (const quest of questsMap.values()) {
    for (const release of quest.releases) {
      for (const qst of release.resources.filter(r => r.endsWith('.qst'))) {
        const path = `${quest.id}/${release.name}/${qst}`;
        await cb({quest, release, qst, path});
      }
    }
  }
}

async function main() {
  if (OFFICIAL_SYNC) {
    execFileSync('s3cmd', ['sync', 's3://zc-data/', '--no-preserve', `${DB}/`], {stdio: 'inherit'});
  } else if (OFFICIAL) {
    // When updating, we only need a few files.
    // We do not sync everything in CI because we don't need a full copy to run this script.
    const files = [
      'authors.json',
      'cache.json',
      'manifest.json',
      'quest_db_donotexist.json',
    ];
    for (const file of files) {
      execFileSync('s3cmd', ['sync', `s3://zc-data/${file}`, '--no-preserve', `${DB}/${file}`], {stdio: 'inherit'});
    }
  }

  loadQuests();
  loadAuthors();
  loadCache();

  // for (const quest of questsMap.values()) {
  //   for (const release of quest.releases) {
  //     for (const qst of release.resources.filter(r => r.endsWith('.qst'))) {
  //       const path = `${DB}/${quest.id}/${release.name}/${qst}`;
  //       await uncompressQstAndGzip(path);
  //     }
  //   }
  // }
  // process.exit(0);

  const browser = await puppeteer.launch({headless: 'new'});
  const page = await browser.newPage();

  // Needed for scraping author ids.
  const isLoggedIn = PZC_UN && PZC_PW;
  if (isLoggedIn) {
    await page.goto('https://www.purezc.net');
    await page.click('#sign_in');
    await page.waitForSelector('#ips_username', {visible: true});
    await page.type('#ips_username', PZC_UN);
    await page.type('#ips_password', PZC_PW);
    await page.evaluate(() => document.querySelector('#login').submit());
    await page.waitForNetworkIdle();
  }

  if (process.env.DM) {
    for (const [author, quests] of groupByAuthor().entries()) {
      if (!author.id) continue;
      if (author.sentApprovalMessage) continue;
      if (author.id !== 293775) continue;
      continue;
      console.log('ok');
      console.log('DM', author);

      const relevantQuests = quests.filter(q => q.approval === 'auto' || q.approval === 'pending');
      if (relevantQuests.length === 0) continue;

      function getQstOpenLink(quest: QuestManifest) {
        const qsts = quest.releases[0].resources.filter(r => r.endsWith('.qst'));

        let href;
        if (qsts.length === 1) href = `https://web.zquestclassic.com/play/?open=${quest.id}`;
        else href = `https://web.zquestclassic.com/play/?open=${quest.defaultPath}`;

        const url = new URL(href);
        url.searchParams.set('name', quest.name.replace(/['!?]/g, '').replace(/[\s:&]+/g, '-').replace(/[-]+/g, '-'));
        url.search = decodeURIComponent(url.search);
        return url.toString();
      }

      const title = 'Requesting approval to publish your quest on web.zquestclassic.com';
      const message = `
(FYI: This message was sent with the help of an automated script)

TLDR: Would you like the developers of ZC to make your quest available on the Web version of ZC at [url=https://web.zquestclassic.com]https://web.zquestclassic.com[/url]?

In April 2022 I ported ZC to the Web. My goal was to make quests easier than ever to play and share with others. I hosted this Web version of ZC on my personal website, and included a mirror of every quest from the PureZC database. I never asked the quest authors permission to host their quests on my website, and I'd like to rectify that mistake now that we are launching a new website for ZQuest Classic: [url=https://zquestclassic.com]https://zquestclassic.com[/url]

Please respond to this PM indicating if you'd like (or would not like) your quest(s) to be available on the ZC website:

[LIST]
${relevantQuests.map(q => `[*]${q.name} (by ${q.authors.map(a => a.name).join(', ')}): [url=${q.projectUrl}]purezc.net[/url] | [url=${getQstOpenLink(q)}]web.zquestclassic.com[/url][/*]`).join('\n')}
[/LIST]

Some Q/As I've anticipated:

Q: How can I share my quest if hosted on the Web version of ZC?
A: By just sharing a link, for example: [url=https://web.zquestclassic.com/play/?open=quests/purezc/773]https://web.zquestclassic.com/play/?open=quests/purezc/773[/url], you can find your quest in the "Quest List" of the Web version, click on "Play", then just share that URL. See the links above that would be used for your quest (NOTE: may not work until you give your approval).

Q: What about enhanced music?
A: External links to music zips listed in a quest project page are automatically extracted and hosted on the ZC Web version. When needed by the engine, the Web version will download music files one at a time on the browser client. You may continue to utilize external file sharing services to host external music - if for some reason you'd like us to host a music zip directly just reach out.

Q: Can I change my answer later?
A: Yes! Just message me. In the future, I / PZC admin will add a field to the quest project page to streamline the approval process. But until then, a PM is fine.

Q: What if I update my quest on PureZC?
A: The Web version will automatically grab the newer version, and new save files will use that. Currently existing save files in the Web version will stick to the previous version, but I'm working on a way to easily update which qst a save slot points to. If your quest would have breaking changes such that this would break a player's playthrough, please utilize the version fields found in the Header dialog in the ZC quest editor to indicate that.

Q: Why is the web version laggy?
A: It's still in an experimental phase. I've found quests to be very playable, as long as it isn't one of the more demanding scripted ones and your machine is somewhat powerful. A goal over the next year is to improve the performance of the engine and the Web version so that all quests run smoothly on most hardware, including mobile devices.

Q: Will the developers ever make money off my quest being available on the Web version of ZC?
A: No.
      `.trim();

      await page.goto(`https://www.purezc.net/forums/index.php?showuser=${author.id}`);
      await page.click('.pm_button');
      await page.waitForSelector('input.input_submit[name="send_msg"]', {visible: true});
      await page.type('input[name="msg_title"]', title);
      await page.$eval('textarea[name="Post"]', (el, text) => el.value = text, message);

      await Promise.all([
        page.click('input.input_submit[name="send_msg"]'),
        page.waitForNavigation(),
      ]);
      author.sentApprovalMessage = true;
      saveAuthors();
      break;
    }

    process.exit(0);
  }

  if (process.env['APPROVALS']) {
    // Mark each quest with an `approval` bit.
    // If author has not been active since 2023 Jan 1, set to `auto`.
    const questsByAuthor = groupByAuthor();

    for (const [author, quests] of questsByAuthor) {
      console.log(author, quests.length);
    }

    for (const quest of questsMap.values()) {
      if (quest.approval !== 'pending') continue;

      let mostRecentAuthor = null;
      for (const author of quest.authors) {
        if (!author.id) continue;

        const lastLogin = authorsMap.get(author.id)?.lastLogin;
        if (!lastLogin) continue;

        if (!mostRecentAuthor?.id) {
          mostRecentAuthor = author;
          continue;
        }

        const prevLastLogin = authorsMap.get(mostRecentAuthor.id)?.lastLogin as Date;
        if (prevLastLogin < lastLogin) {
          mostRecentAuthor = author;
          continue;
        }
      }

      const lastLogin = mostRecentAuthor?.id ? authorsMap.get(mostRecentAuthor.id)?.lastLogin as Date : null;
      const hasActiveAuthor = lastLogin && lastLogin >= new Date('2023-01-01');
      if (!hasActiveAuthor) {
        quest.approval = 'auto';
        console.log(quest.id, quest.name, mostRecentAuthor, lastLogin);
      }
    }
  } else if (ONE_SHOT) {
    await processId(page, ONE_SHOT);
  } else {
    const start = START || 1;
    const max = MAX || await getLatestPzcId(page);
    console.log(`processing ${max - start + 1} quests`);
    for (let i = start; i <= max; i++) {
      const id = `quests/purezc/${i}`;
      if (ONLY_NEW && questsMap.has(id)) {
        continue;
      }
      if (doNotExist.includes(id)) {
        continue;
      }
  
      try {
        console.log(`[${id}]`);
        await processId(page, i);
      } catch (e) {
        console.error(`[${id}]`, e);
        console.error(`[${id}]`, 'will try again next run');
      }
  
      if (i % 10 === 0) saveQuests();
    }
  }

  if (isLoggedIn) {
    for (const quest of questsMap.values()) {
      for (const author of quest.authors) {
        if (!author.id || authorsMap.has(author.id)) continue;
  
        let lastLogin;
        const id = author.id;
        await page.goto(`https://www.purezc.net/forums/index.php?showuser=${id}`);
        const lastLoginRaw = await page.evaluate(() => {
          const el = document.querySelector('#user_info_cell .desc');
          if (!el?.textContent) throw new Error();
          if (!el.textContent?.includes('Last Active ')) throw new Error();
          return el.textContent.replace('Last Active ', '');
        });
        if (lastLoginRaw.includes('Today') || lastLoginRaw.includes('Yesterday')) lastLogin = new Date();
        else if (lastLoginRaw.match(/(minute|hour)s? ago/)) lastLogin = new Date();
        else if (!lastLoginRaw.includes('Private')) lastLogin = new Date(lastLoginRaw);

        console.log(id, {id, name: author.name, lastLogin});
        authorsMap.set(id, {id, name: author.name, lastLogin});
        saveAuthors();
      }
    }
  }

  console.log('\nshutting down');
  saveQuests();

  await browser.close();
  if (OFFICIAL) {
    console.log('\nsyncing s3');
    execFileSync('s3cmd', [
      'sync',
      '--no-preserve',
      '--acl-public',
      `${DB}/`,
      's3://zc-data/',
    ], {stdio: 'inherit'});

    console.log('\nadding encoding headers');
    await forEveryQst(({path}) => {
      if (cache.encodingHeaders[path]) return;
      const diskPath = `${DB}/${path}.gz`;
      if (!fs.existsSync(diskPath)) return;

      execFileSync('s3cmd', [
        'modify',
        '--add-header=Content-Encoding:gzip',
        `s3://zc-data/${path}.gz`,
      ], {stdio: 'inherit'});
      cache.encodingHeaders[path] = true;
    });

    console.log('\nadding size headers');
    await forEveryQst(({path}) => {
    if (cache.sizeHeaders[path]) return;

      const diskPath = `${DB}/${path}.gz`;
      if (!fs.existsSync(diskPath)) return;

      const gzipText = execFileSync('gzip', [
        '-l', diskPath,
      ], {encoding: 'utf-8'});
      const size = Number(gzipText.trim().split('\n')[1].trim().split(/\s+/)[1]);
      console.log('uncompressed size', `${path}.gz`, size);
      if (!Number.isFinite(size) || size <= 0) return;

      execFileSync('s3cmd', [
        'modify',
        `--add-header=X-Amz-Meta-Inflated-Content-Size:${size}`,
        `s3://zc-data/${path}.gz`,
      ], {stdio: 'inherit'});
      cache.sizeHeaders[path] = true;
    });
  }

  saveCache();
  if (OFFICIAL) {
    execFileSync('s3cmd', [
      'put',
      `${DB}/cache.json`,
      's3://zc-data/cache.json',
    ], {stdio: 'inherit'});
  }
}

main();
