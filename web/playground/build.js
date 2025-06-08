// Don't run directly, instead:
//
// 1. Follow the instructions for configuring a web build in "docs/building.md"
// 2. build via cmake target "web_zscript_playground"

import fs from 'fs';
import path from 'path';
import esbuild from 'esbuild';

const minify = process.argv.includes('--minify');
const BUILD_FOLDER = process.argv[process.argv.indexOf('--build_folder') + 1];
const dist = `${BUILD_FOLDER}/playground`;

function copy(file, dest = dist) {
  fs.copyFileSync(file, `${dest}/${path.basename(file)}`);
}

await esbuild.build({
  entryPoints: {
    playground: './playground.ts',
    'playground.worker': './playground.worker.ts',
  },
  entryNames: '[name]',
  bundle: true,
  minify,
  // TODO: can't get ESM to work from Firefox. Even when importing worker as type: module, it
  // cannot use `import.meta.url` ...
  // format: 'esm',
  outdir: dist,
  loader: {
    '.ttf': 'file',
  },
  sourcemap: true,
  alias: {
    'onigasm': 'vscode-oniguruma',
    'path': './path-shim.js'
  },
  external: [
    './zscript-playground.data.js',
    '../zscript.mjs',
  ],
});

await esbuild.build({
  entryPoints: {
    'editor.worker': 'monaco-editor/esm/vs/editor/editor.worker.js',
  },
  entryNames: '[name]',
  bundle: true,
  minify,
  outdir: dist,
  sourcemap: true,
});

fs.cpSync(`${import.meta.dirname}/themes`, `${dist}/themes`, { recursive: true });
copy(`${import.meta.dirname}/index.html`);
copy(`${import.meta.dirname}/../../vscode-extension/syntaxes/zscript.tmLanguage.json`);
copy(`${import.meta.dirname}/../node_modules/vscode-oniguruma/release/onig.wasm`);

// This "error" is just noise.
const needle = `console.error("[4] - Grammar is in an endless loop - Grammar is not advancing, nor is it pushing/popping")`;
let js = fs.readFileSync(`${dist}/playground.js`, 'utf-8');
js = js.replace(needle, `0&&${needle}`);
fs.writeFileSync(`${dist}/playground.js`, js);

// Until can bundle as ESM, gotta hack the IIFE output.
js = fs.readFileSync(`${dist}/playground.worker.js`, 'utf-8');
js = js.replaceAll('import_meta.url', 'location.href');
fs.writeFileSync(`${dist}/playground.worker.js`, js);
