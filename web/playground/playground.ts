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
// File names (as stored in the gist) of the editor's files in the gist currently
// loaded or last uploaded.
let gistFileNames = new Set<string>();

const openBtnEl = findElement('.open-btn') as HTMLAnchorElement;
const shareBtnEl = findElement('.share-btn');
const embedBtnEl = findElement('.embed-btn');
const zasmBtnEl = findElement('.zasm-btn');
const zasmPaneEl = findElement('.zasm-pane');

async function initWorker() {
  ZScriptWorker.worker.addEventListener('message', e => {
    const id = e.data.id;
    const entry = ZScriptWorker.commandPromiseResolvers.get(id);
    if (!entry) return;
    ZScriptWorker.commandPromiseResolvers.delete(id);
    if (e.data.error) entry.reject(new Error(e.data.error));
    else entry.resolve(e.data.result);
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

  deleteFile(path: string): Promise<void> {
    return ZScriptWorker.sendCommand('delete', { path });
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

  // Toggle line comment(s) for the current selection with Ctrl/Cmd + Shift + /.
  editor.addAction({
    id: 'zscript.toggleLineComment',
    label: 'Toggle Line Comment',
    keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.Slash],
    run: (ed) => { ed.getAction('editor.action.commentLine')?.run(); },
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
  // The tab may have been closed while the compile was in flight.
  if (model.isDisposed()) return;

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
  // A debounced content change can fire after the tab was closed.
  if (model.isDisposed()) return;

  try {
    await ZScriptWorker.writeFile(model.uri.path, model.getValue());
    // Closing the tab meanwhile deletes the file, so there is nothing to compile.
    if (model.isDisposed()) return;
    await recompileModel(model);
  } catch (err: any) {
    console.error(err);
    logger.warn(`failed to compile ${basename(model.uri)}: ${err.message ?? err}`);
    return;
  }
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
  // Only the files loaded into the editor are managed by it; anything else in
  // the gist (ex: a readme) is left alone when updating it.
  const loadedNames = new Set<string>();
  if (data.files['manifest.json']) {
    loadedNames.add('manifest.json');
    const manifest = JSON.parse(contents[keys.indexOf('manifest.json')]);
    for (const [name, path] of Object.entries(manifest.files as Record<string, string>)) {
      const index = keys.indexOf(name);
      if (index === -1) {
        logger.warn(`gist is missing file listed in manifest: ${name}`);
        continue;
      }
      files.set(path, contents[index]);
      loadedNames.add(name);
    }
  } else {
    for (let i = 0; i < keys.length; i++) {
      const key = keys[i];
      if (key.endsWith('.zs') || key.endsWith('.zh') || key.endsWith('.z')) {
        files.set(keys[i], contents[i]);
        loadedNames.add(key);
      }
    }
  }

  contentGistId = gist;
  gistFileNames = loadedNames;
  await checkUserOwnsGist(gist, data);
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
      if (!response.ok) {
        logger.error(`error loading manifest: ${await response.text()}`);
        return null;
      }

      const manifest = await response.json();
      const entries = [...Object.entries(manifest.files as Record<string, string>)];
      const resolvedEntries = await Promise.all(entries.map(async ([fileUrl, path]) => {
        // Relative file urls are relative to the manifest, not to the playground.
        const fileResponse = await fetch(new URL(fileUrl, response.url || url));
        const content = await fileResponse.text();
        if (!fileResponse.ok) throw new Error(`error loading ${fileUrl}: ${content}`);
        return { content, path };
      }));

      const files = new Map<string, string>();
      for (const {content, path} of resolvedEntries) {
        files.set(path, content);
      }
      contentUrl = url;
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

// Returns the open model for a uri, or a read-only one loaded from the compiler's
// file system (ex: a bundled header), or null if neither exists.
async function getOrLoadModel(uri: monaco.Uri) {
  const existing = monaco.editor.getModel(uri);
  if (existing) return existing;

  const content = await ZScriptWorker.readFile(uri.path).catch(() => null);
  if (content === null) return null;
  // The model may have been created while the read was in flight.
  return monaco.editor.getModel(uri) ?? createModel(content, uri);
}

const tabsEl = findElement('.tabs');
const activeTabs = new Map<monaco.Uri, HTMLElement>();

async function checkUserOwnsGist(gist: string, data?: any) {
  const user = await auth.getGithubUserIfLoggedIn();
  if (!user) return false;

  if (!data) data = await fetch(`https://api.github.com/gists/${gist}`).then(r => r.json());
  ownsGist = String(data.owner?.id) === user.uid;
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
  // A closed file is gone (it won't be shared), so it shouldn't stay includable
  // either. Read-only models are bundled headers, which must stay.
  if (!getState(model).readOnly) {
    ZScriptWorker.deleteFile(model.uri.path).catch(err => console.error(err));
  }

  if (editor.getModel() == model) {
    editor.setModel(null);
    // The only sibling that isn't a tab is the "+" button, which has no uri.
    const nextTabEl = (tabEl.previousElementSibling ?? tabEl.nextElementSibling) as HTMLElement | null;
    const nextModel = nextTabEl?.dataset.uri && monaco.editor.getModel(monaco.Uri.parse(nextTabEl.dataset.uri));
    if (nextModel) {
      openModel(nextModel);
    } else {
      zasmPaneEl.textContent = '';
    }
  } else if (!getState(model).readOnly && editor.getModel()) {
    // The open file may have included the closed one.
    onContentUpdated(editor.getModel()!);
  }

  tabEl.remove();
  activeTabs.delete(model.uri);
  modelStates.delete(model);
  model.dispose();
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

  updateZasm(model);
  onContentUpdated(model);
}

function updateZasm(model: monaco.editor.ITextModel) {
  // Compiles finish asynchronously, so this may be called for a tab that is no
  // longer the active one.
  if (editor.getModel() !== model) return;

  const zasm = getState(model).zasm ?? 'Loading...';
  zasmPaneEl.textContent = `ZASM for ${basename(model.uri)}:\n\n${zasm}`;
}

async function gistRequest(method: string, url: string, body: object) {
  const send = (token: string) => fetch(url, {
    method,
    headers: new Headers({ Authorization: `token ${token}` }),
    body: JSON.stringify(body),
  });

  let response = await send(await auth.getAccessToken());
  if (response.status === 401) {
    // The stored token was revoked or expired, so sign in again.
    await auth.signOut();
    response = await send(await auth.getAccessToken());
  }
  return response;
}

async function share() {
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
    files: files as Record<string, { content: string } | null>,
  };

  if (contentGistId && (ownsGist || await checkUserOwnsGist(contentGistId))) {
    // Files closed since the gist was loaded should be removed from it too.
    for (const name of gistFileNames) {
      if (!(name in files)) body.files[name] = null;
    }

    const response = await gistRequest('PATCH', `https://api.github.com/gists/${contentGistId}`, body);
    const json = await response.json();
    if (response.ok) {
      navigator.clipboard.writeText(location.href);
      logger.log('Updated! Shareable URL copied to clipboard.');
    } else {
      logger.error(`gist error: ${json.message}`);
      return;
    }
  } else {
    const response = await gistRequest('POST', 'https://api.github.com/gists', body);
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

  gistFileNames = new Set(Object.keys(files));
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

  tabsEl.addEventListener('click', async (e) => {
    if (!(e.target instanceof HTMLElement)) return;

    if (e.target.closest('.tabs__add')) {
      const name = window.prompt('File name: (ex: script.zs)');
      if (name === null) return;

      const uri = monaco.Uri.file(name.trim() || 'script.zs');
      // Opening a bundled header (ex: include/std.zh) must not replace it with
      // an empty file, so it is shown read-only like go to definition does.
      let model = await getOrLoadModel(uri);
      if (!model) {
        model = createModel('', uri);
        getState(model).readOnly = false;
      }
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
    shareBtnEl.addEventListener('click', () => {
      share().catch(err => {
        console.error(err);
        logger.warn(`failed to share: ${err.message ?? err}`);
      });
    });
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
          identifier.loc.line + 1,
          identifier.loc.character + 1,
          identifier.loc.line + 1,
          identifier.loc.character + 1 + identifier.loc.length,
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
      if (!await getOrLoadModel(uri)) return;

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
        editor.setSelection(selectionOrPosition);
        editor.revealRangeNearTop(selectionOrPosition);
      } else if (monaco.Position.isIPosition(selectionOrPosition)) {
        editor.setPosition(selectionOrPosition);
        editor.revealPositionNearTop(selectionOrPosition);
      }
      editor.focus();
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
    if (models.length) openModel(models[0]);
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

monaco.languages.setLanguageConfiguration('zscript', {
  comments: {
    lineComment: '//',
    blockComment: ['/*', '*/'],
  },
});

main().catch(e => {
  console.error(e);
  document.body.textContent = e.message;
});
