import puppeteer from 'puppeteer';
import statikk from 'statikk';
import { setupConsoleListener } from './utils.js';

const buildFolder = process.argv[2];
const target = process.argv[3];
const args = process.argv.slice(4);

const server = statikk({
  coi: true,
  root: `${buildFolder}/packages/web`,
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
  console.log(openUrl.href);
  const browser = await puppeteer.launch({
    headless: openUrl.searchParams.has('headless') ? 'new' : false,
  });
  const page = await browser.newPage();
  let hasExited = false;
  let exitCode = -1;
  const onClose = async () => {
    await browser.close();
    await server.server.close();
  };

  const consoleListener = setupConsoleListener(page);
  page.on('pageerror', e => {
    process.stderr.write(e.toString());
    process.stderr.write(e.stack);
    process.stderr.write('\n');
    hasExited = true;
  });

  page.on('console', async (e) => {
    const type = e.type();
    const args = await Promise.all(e.args().map(arg => page.evaluate(arg => {
      if (arg instanceof Error) {
        return arg.message + '\n' + arg.stack;
      }
      return arg;
    }, arg).catch((e) => {
      console.error('error in run_web_version.js', e);
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
      onClose();
      process.exit(1);
    }
  });

  await page.goto(openUrl, {
    waitUntil: 'networkidle2',
  });

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

  await onClose();

  return exitCode;
}

process.exit(await run());
