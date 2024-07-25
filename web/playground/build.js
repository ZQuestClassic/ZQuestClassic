import fs from 'fs';
import path from 'path';
import esbuild from 'esbuild';

const BUILD_FOLDER = process.env.BUILD_FOLDER ?? `${import.meta.dirname}/../../build_emscripten/Debug`;
const dist = `${import.meta.dirname}/dist`;

function copy(file, dest = dist) {
  fs.copyFileSync(file, `${dest}/${path.basename(file)}`);
}

copy(`${BUILD_FOLDER}/zscript.data.js`, import.meta.dirname);
copy(`${BUILD_FOLDER}/zscript.mjs`, import.meta.dirname);

await esbuild.build({
  entryPoints: {
    playground: './playground.ts',
    'playground.worker': './playground.worker.ts',
  },
  entryNames: '[name]',
  bundle: true,
  // TODO: can't get ESM to work from Firefox. Even when importing worker as type: module, it
  // cannot use `import.meta.url` ...
  // format: 'esm',
  outdir: './dist',
  loader: {
    '.ttf': 'file',
  },
  sourcemap: true,
  alias: {
    'onigasm': 'vscode-oniguruma',
  },
});

await esbuild.build({
  entryPoints: {
    'editor.worker': 'monaco-editor/esm/vs/editor/editor.worker.js',
  },
  entryNames: '[name]',
  bundle: true,
  outdir: './dist',
  sourcemap: true,
});

fs.cpSync(`${import.meta.dirname}/themes`, `${dist}/themes`, { recursive: true });
copy(`${import.meta.dirname}/index.html`);
copy(`${import.meta.dirname}/../../vscode-extension/syntaxes/zscript.tmLanguage.json`);
copy(`${import.meta.dirname}/../node_modules/vscode-oniguruma/release/onig.wasm`);
copy(`${BUILD_FOLDER}/zscript.data`);
copy(`${BUILD_FOLDER}/zscript.wasm`);

// This "error" is just noise.
const needle = `console.error("[4] - Grammar is in an endless loop - Grammar is not advancing, nor is it pushing/popping`;
let js = fs.readFileSync(`${dist}/playground.js`, 'utf-8');
js = js.replace(needle, `//${needle}`);
fs.writeFileSync(`${dist}/playground.js`, js);

// Until can bundle as ESM, gotta hack the IIFE output.
js = fs.readFileSync(`${dist}/playground.worker.js`, 'utf-8');
js = js.replaceAll('import_meta.url', 'location.href');
fs.writeFileSync(`${dist}/playground.worker.js`, js);
