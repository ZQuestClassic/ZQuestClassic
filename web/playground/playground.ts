import * as monaco from 'monaco-editor';

import { loadWASM } from 'vscode-oniguruma'
import { Registry } from 'monaco-textmate'
import { wireTmGrammars } from 'monaco-editor-textmate'
import * as defaultTheme from './themes/solarized-dark.json';
import { FirebaseAuth } from './firebase-auth.js';
import { Logger } from './logger.js';

interface DocumentMetadata {
  identifiers: Array<{
    symbol: string;
    loc: {
      line: number;
      character: number;
      length: number;
    };
  }>;
  symbols: Record<string, {
    doc: string;
    loc: {
      uri: string;
      range: {
        start: { line: number, character: number },
        end: { line: number, character: number },
      };
    }
  }>;
}

interface ModelState {
  metadata?: DocumentMetadata;
  zasm?: string;
  view?: monaco.editor.ICodeEditorViewState | null;
  readOnly: boolean;
}

const isFramed = window !== window.parent;
const logger = new Logger(findElement('#zc-log'));
const auth = new FirebaseAuth();
const modelStates = new Map<monaco.editor.ITextModel, ModelState>();
let editor: monaco.editor.IStandaloneCodeEditor;
let contentGistId: string;
let contentUrl: string;
let ownsGist = false;

const openBtnEl = findElement('.open-btn') as HTMLAnchorElement;
const shareBtnEl = findElement('.share-btn');
const embedBtnEl = findElement('.embed-btn');
const zasmBtnEl = findElement('.zasm-btn');
const zasmPaneEl = findElement('.zasm-pane');

async function initWorker() {
  ZScriptWorker.worker.addEventListener('message', e => {
    const id = e.data.id;
    const { resolve, reject } = ZScriptWorker.commandPromiseResolvers.get(id);
    if (e.data.error) reject(new Error(e.data.error));
    else resolve(e.data.result);
  });
  try {
    await ZScriptWorker.sendCommand('init', {});
  } catch (err: any) {
    throw new Error(`failed to start zscript worker: ${err.toString()}`);
  }
}

const ZScriptWorker = {
  worker: new Worker('./playground.worker.js'),
  commandPromiseResolvers: new Map(),
  nextId: 1,

  sendCommand(type: string, data: object) {
    const id = ZScriptWorker.nextId++;
    return new Promise<any>((resolve, reject) => {
      ZScriptWorker.commandPromiseResolvers.set(id, { resolve, reject });
      ZScriptWorker.worker.postMessage({ id, type, data });
    });
  },

  setIncludepaths(includepaths: string): Promise<void> {
    return ZScriptWorker.sendCommand('includepaths', { includepaths });
  },

  writeFile(path: string, code: string): Promise<void> {
    return ZScriptWorker.sendCommand('write', { path, code });
  },

  readFile(path: string): Promise<string> {
    return ZScriptWorker.sendCommand('read', { path });
  },

  compile(path: string): Promise<{ metadata: DocumentMetadata, diagnostics: any[], zasm: string }> {
    return ZScriptWorker.sendCommand('compile', { path });
  },
};

async function loadEditor(themeName: string) {
  const [_, tmText] = await Promise.all([
    fetch('onig.wasm').then(r => r.arrayBuffer()).then(loadWASM),
    fetch('zscript.tmLanguage.json').then(r => r.text()),
  ]);

  const registry = new Registry({
    getGrammarDefinition: async () => {
      return {
        format: 'json',
        content: tmText,
      }
    }
  });
  await loadTheme(themeName);
  editor = monaco.editor.create(findElement('.current-model'), {
    model: null,
    theme: themeName,
    automaticLayout: true,
    stickyScroll: { enabled: false },
    renderValidationDecorations: 'on',
  });

  const languages = new Map([['zscript', 'source.zscript']]);
  await wireTmGrammars(monaco, registry, languages, editor);
}

function getState(model: monaco.editor.ITextModel) {
  let state = modelStates.get(model);
  if (!state) {
    state = { readOnly: true };
    modelStates.set(model, state);
  }
  return state;
}

const defaultContent = `#include "std.zh"

ffc script OldMan
{
    void run()
    {
        printf("it's dangerous to go alone, take this");
    }
}
`;

const debounce = (callback: (...args: any[]) => any, wait: number) => {
  let timeoutId: number | undefined;
  return (...args: any[]) => {
    window.clearTimeout(timeoutId);
    timeoutId = window.setTimeout(() => {
      callback(...args);
    }, wait);
  };
}

const themesLoaded = new Set<string>();
async function loadTheme(themeName: string) {
  if (themesLoaded.has(themeName)) {
    return;
  }

  let theme: monaco.editor.IStandaloneThemeData;
  if (themeName === 'solarized-dark-color-theme') {
    theme = defaultTheme as any;
  } else {
    const url = new URL(`themes/${themeName}.json`, document.location.href);
    theme = await fetch(url).then(r => r.json());
  }
  monaco.editor.defineTheme(themeName.replaceAll('_', '-'), theme);
  themesLoaded.add(themeName);
}

function findElement(selector: string) {
  const el = document.querySelector(selector);
  if (!el) throw new Error(`no element found for selector: ${selector}`);
  return el as HTMLElement;
}

async function recompileModel(model: monaco.editor.ITextModel) {
  const result = await ZScriptWorker.compile(model.uri.path);
  const state = getState(model);
  state.metadata = result.metadata;
  state.zasm = result.zasm;

  const markers = result.diagnostics.map(d => {
    const severity = d.severity === 1 ? monaco.MarkerSeverity.Error : monaco.MarkerSeverity.Warning;
    return {
      severity,
      message: d.message,
      startLineNumber: d.range.start.line + 1,
      startColumn: d.range.start.character + 1,
      endLineNumber: d.range.end.line + 1,
      endColumn: d.range.end.character + 1,
    };
  });
  monaco.editor.setModelMarkers(model, '', markers);
}

async function onContentUpdated(model: monaco.editor.ITextModel) {
  await ZScriptWorker.writeFile(model.uri.path, model.getValue());
  await recompileModel(model);
  updateZasm(model);
}

async function loadFromGist(gist: string) {
  const files = new Map<string, string>();

  const response = await fetch(`https://api.github.com/gists/${gist}`);
  const data = await response.json();
  if (!response.ok) {
    logger.error(`error loading content: ${data.message}`);
    return null;
  }

  const contentPromises: Array<Promise<string>> = [];
  for (const file of Object.values(data.files) as any[]) {
    if (file.truncated) {
      contentPromises.push(fetch(file.raw_url).then(r => r.text()));
    } else {
      contentPromises.push(Promise.resolve(file.content));
    }
  }
  const contents = await Promise.all(contentPromises);
  let keys = Object.keys(data.files);
  if (data.files['manifest.json']) {
    const manifest = JSON.parse(contents[keys.indexOf('manifest.json')]);
    for (const [name, path] of Object.entries(manifest.files as Record<string, string>)) {
      const content = contents[keys.indexOf(name)];
      files.set(path, content);
    }
  } else {
    for (let i = 0; i < keys.length; i++) {
      const key = keys[i];
      if (key.endsWith('.zs') || key.endsWith('.zh') || key.endsWith('.z')) {
        files.set(keys[i], contents[i]);
      }
    }
  }

  contentGistId = gist;
  await checkUserOwnsGist(gist);
  return files;
}

async function loadFiles() {
  const params = new URLSearchParams(location.search);
  const gist = params.get('gist');
  const url = params.get('url');
  const data = params.get('data');
  const fname = params.get('fname') ?? 'main.zs';

  if (gist) {
    const gistFiles = await loadFromGist(gist);
    if (gistFiles) return gistFiles;
    return null;
  } else if (url) {
    const response = await fetch(url);

    if (url.endsWith('.json')) {
      const manifest = await response.json();
      const entries = [...Object.entries(manifest.files as Record<string, string>)];
      const resolvedEntries = await Promise.all(entries.map(async entry => ({
        content: await (await fetch(entry[0])).text(),
        path: entry[1],
      })));

      const files = new Map<string, string>();
      for (const {content, path} of resolvedEntries) {
        files.set(path, content);
      }
      return files;
    } else {
      const content = await response.text();
      if (!response.ok) {
        logger.error(`error loading content: ${content}`);
        return null;
      }

      const files = new Map<string, string>();
      files.set(fname, content);
      contentUrl = url;
      return files;
    }
  } else if (data) {
    const files = new Map<string, string>();
    files.set(fname, data);
    return files;
  }

  const files = new Map<string, string>();
  files.set(fname, defaultContent);
  return files;
}

function findIdentifierAndSymbol(metadata: DocumentMetadata, position: monaco.Position) {
  const identifier = metadata.identifiers.find(({ loc }) => {
    if (loc.line + 1 !== position.lineNumber) return false;
    if (loc.character + 1 > position.column) return false;
    if (loc.character + loc.length < position.column) return false;
    return true;
  });
  if (!identifier) return;

  const symbol = metadata.symbols[identifier.symbol];
  return { identifier, symbol };
}

function createModel(content: string, uri: monaco.Uri) {
  const model = monaco.editor.createModel(content, 'zscript', uri);
  model.onDidChangeContent(debounce(() => onContentUpdated(model), 300));
  return model;
}

const tabsEl = findElement('.tabs');
const activeTabs = new Map<monaco.Uri, HTMLElement>();

async function checkUserOwnsGist(gist: string) {
  const user = await auth.getGithubUserIfLoggedIn();
  if (!user) return false;

  const data = await fetch(`https://api.github.com/gists/${gist}`).then(r => r.json());
  ownsGist = String(data.owner.id) === user.uid;
  findElement('.share-btn').textContent = ownsGist ? 'Update Gist' : 'Share';
  return ownsGist;
}

function basename(uri: monaco.Uri) {
  const parts = uri.path.split('/');
  return parts[parts.length - 1];
}

function createTab(model: monaco.editor.ITextModel) {
  const tabEl = document.createElement('div');
  tabsEl.insertBefore(tabEl, tabsEl.lastElementChild);
  tabEl.dataset.uri = model.uri.toString();
  tabEl.textContent = basename(model.uri);
  tabEl.className = 'tab';

  const closeEl = document.createElement('div');
  tabEl.append(closeEl);
  closeEl.classList.add('tab__close');
  closeEl.textContent = 'x';

  activeTabs.set(model.uri, tabEl);
  return tabEl;
}

function closeTab(model: monaco.editor.ITextModel, tabEl: HTMLElement) {
  if (editor.getModel() == model) {
    editor.setModel(null);
    const nextTabEl = (tabEl.previousElementSibling ?? tabEl.nextElementSibling) as HTMLElement;
    const nextModel = nextTabEl && monaco.editor.getModel(monaco.Uri.parse(nextTabEl.dataset.uri ?? ''));
    if (nextModel) {
      openModel(nextModel);
    }
  }

  tabEl.remove();
  activeTabs.delete(model.uri);
}

function openModel(model: monaco.editor.ITextModel) {
  const currentModel = editor.getModel();
  if (currentModel) {
    const state = getState(currentModel);
    state.view = editor.saveViewState();
  }

  editor.setModel(model);
  const state = getState(model);
  if (state.view) {
    editor.restoreViewState(state.view);
  }
  editor.updateOptions({ readOnly: state.readOnly });

  let tabEl = activeTabs.get(model.uri);
  if (!tabEl) {
    tabEl = createTab(model);
  }
  tabsEl.querySelector('.tab--selected')?.classList.remove('tab--selected');
  tabEl.classList.add('tab--selected');

  onContentUpdated(model);
}

function updateZasm(model: monaco.editor.ITextModel) {
  const zasm = getState(model).zasm ?? 'Loading...';
  zasmPaneEl.textContent = `ZASM for ${basename(model.uri)}:\n\n${zasm}`;
}

async function share() {
  const token = await auth.getAccessToken();
  const models = [...modelStates.keys()].filter(m => !getState(m).readOnly && m.getValue());
  const files = {} as Record<string, { content: string }>;

  const manifest = {
    files: {} as Record<string, string>,
  };
  for (const model of models) {
    let name = model.uri.path;
    if (name.startsWith('/')) name = name.substring(1);
    name = name.replaceAll('/', '_');
    manifest.files[name] = model.uri.path;

    files[name] = {
      content: model.getValue(),
    };
  }
  files['manifest.json'] = { content: JSON.stringify(manifest, null, 2) };

  const body = {
    description: 'ZScript',
    public: false,
    files,
  };

  if (contentGistId && (ownsGist || await checkUserOwnsGist(contentGistId))) {
    const response = await fetch(`https://api.github.com/gists/${contentGistId}`, {
      method: 'PATCH',
      headers: new Headers({ Authorization: `token ${token}` }),
      body: JSON.stringify(body),
    });
    const json = await response.json();
    if (response.ok) {
      navigator.clipboard.writeText(location.href);
      logger.log('Updated! Shareable URL copied to clipboard.');
    } else {
      logger.error(`gist error: ${json.message}`);
      return;
    }
  } else {
    const response = await fetch('https://api.github.com/gists', {
      method: 'POST',
      headers: new Headers({ Authorization: `token ${token}` }),
      body: JSON.stringify(body),
    });
    const json = await response.json();
    if (response.ok) {
      contentGistId = json.id;
      history.replaceState({}, '', `?gist=${json.id}`);
      navigator.clipboard.writeText(location.href);
      logger.log('Uploaded! Shareable URL copied to clipboard.');
    } else {
      logger.error(`gist error: ${json.message}`);
      return;
    }
  }

  embedBtnEl.classList.remove('hidden');
  ownsGist = true;
  findElement('.share-btn').textContent = 'Update Gist';
}

export async function main() {
  findElement('#theme').addEventListener('change', async (e) => {
    if (!(e.target instanceof HTMLSelectElement)) return;

    await loadTheme(e.target.value);
    monaco.editor.setTheme(e.target.value.replaceAll('_', '-'));
  });

  zasmBtnEl.addEventListener('click', () => {
    const splitPaneEl = findElement('.split-pane-container');
    splitPaneEl.classList.toggle('show-zasm');
  });

  const [, , files] = await Promise.all([
    loadEditor('solarized-dark-color-theme'),
    initWorker(),
    loadFiles(),
  ]);
  findElement('.loading').classList.add('hidden');
  findElement('.split-pane-container').classList.remove('hidden');

  tabsEl.addEventListener('click', (e) => {
    if (!(e.target instanceof HTMLElement)) return;

    if (e.target.closest('.tabs__add')) {
      const name = window.prompt('File name: (ex: script.zs)');
      if (name === null) return;

      const model = createModel('', monaco.Uri.file(name || 'script.zs'));
      getState(model).readOnly = false;
      openModel(model);
      return;
    }

    const tabEl = e.target.closest('.tab') as HTMLElement;
    if (!tabEl || !tabEl.dataset.uri) return;

    const model = monaco.editor.getModel(monaco.Uri.parse(tabEl.dataset.uri));
    if (!model) return;

    if (e.target.closest('.tab__close')) {
      closeTab(model, tabEl);
    } else {
      openModel(model);
    }
  });

  if (isFramed) {
    openBtnEl.href = location.href;
    openBtnEl.classList.remove('hidden');
  } else {
    shareBtnEl.addEventListener('click', share);
    shareBtnEl.classList.remove('hidden');
  }

  embedBtnEl.addEventListener('click', () => {
    const html = `<iframe width=800 height=800 src="${location.href}"></iframe>`;
    navigator.clipboard.writeText(html);
    logger.log('HTML embed copied to clipboard.');
  });
  if (!isFramed && (contentGistId || contentUrl)) {
    embedBtnEl.classList.remove('hidden');
  }

  monaco.languages.registerHoverProvider('zscript', {
    provideHover: function (model, position) {
      const state = getState(model);
      if (!state.metadata) return;

      const { identifier, symbol } = findIdentifierAndSymbol(state.metadata, position) ?? {};
      if (!identifier || !symbol) return;

      return {
        range: new monaco.Range(
          identifier.loc.line,
          identifier.loc.character,
          identifier.loc.line,
          identifier.loc.character + identifier.loc.length,
        ),
        contents: [
          { value: `**${model.getWordAtPosition(position)?.word}**` },
          {
            value: symbol.doc,
          },
        ],
      };
    },
  });

  monaco.languages.registerDefinitionProvider('zscript', {
    provideDefinition: async (model, position, token) => {
      const state = getState(model);
      if (!state.metadata) return;

      const { identifier, symbol } = findIdentifierAndSymbol(state.metadata, position) ?? {};
      if (!identifier || !symbol) return;

      const uri = monaco.Uri.file(symbol.loc.uri.replace('file://', ''));
      if (!monaco.editor.getModel(uri)) {
        const content = await ZScriptWorker.readFile(uri.path);
        createModel(content, uri);
      }

      return {
        uri,
        range: new monaco.Range(
          symbol.loc.range.start.line + 1,
          symbol.loc.range.start.character + 1,
          symbol.loc.range.end.line + 1,
          symbol.loc.range.end.character + 1,
        ),
      }
    }
  });

  monaco.editor.registerEditorOpener({
    async openCodeEditor(source, resource, selectionOrPosition) {
      const model = monaco.editor.getModel(resource);
      if (!model || model === editor.getModel()) return false;

      openModel(model);
      if (monaco.Range.isIRange(selectionOrPosition)) {
        editor.revealRangeNearTop(selectionOrPosition);
      } else if (monaco.Position.isIPosition(selectionOrPosition)) {
        editor.revealPositionNearTop(selectionOrPosition);
      }
      return true;
    }
  });

  if (files) {
    const includePathsSet = new Set<string>(['include', 'headers', 'scripts']);
    for (const path of files.keys()) {
      const parts = path.split('/');
      for (let i = 0; i < parts.length; i++) {
        const dir = parts.slice(0, i).join('/');
        if (dir) {
          includePathsSet.add(dir);
        }
      }
    }
    await ZScriptWorker.setIncludepaths([...includePathsSet].join(';'));

    const models = [...files.entries()].map(([name, content]) => {
      return createModel(content, monaco.Uri.file(name));
    });
    for (const model of models) {
      getState(model).readOnly = false;
      createTab(model);
      await ZScriptWorker.writeFile(model.uri.path, model.getValue());
    }
    openModel(models[0]);
    await recompileModel(models[0]);
  }
}

self.MonacoEnvironment = {
  getWorkerUrl: function () {
    return './editor.worker.js';
  },
};

monaco.languages.register({
  id: 'zscript',
});

main().catch(e => {
  console.error(e);
  document.body.textContent = e.message;
});
