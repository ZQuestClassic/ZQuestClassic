async function createModule() {
  const [zscriptData, zscriptModule] = [await import('./zscript-playground.data.js'), await import('../zscript.mjs')];
  const Module = zscriptData.default;
  Module.noInitialRun = true;
  await zscriptModule.default(Module);
  Module.compileScript = Module.cwrap('compile_script', 'int', ['string']);
  return Module;
}

function compile(Module: any, filename: string) {
  const consolePath = 'out.txt';
  const result = Module.compileScript(filename);
  const success = result === 0;
  const output = new TextDecoder().decode(Module.FS.readFile(consolePath));
  const { diagnostics, metadata } = JSON.parse(output);
  return { success, diagnostics, metadata };
};

async function setup() {
  let Module: any;

  self.onmessage = async (e) => {
    const { id, type, data } = e.data;

    try {
      if (type === 'init') {
        Module = await createModule();
        self.postMessage({ id });
      } else if (type === 'includepaths') {
        const { includepaths } = data;
        Module.FS.writeFile('includepaths.txt', includepaths);
        self.postMessage({ id });
      } else if (type === 'write') {
        const { path, code } = data;
        Module.FS.mkdirTree(path.split('/').slice(0, -1).join('/'));
        Module.FS.writeFile(path, code);
        self.postMessage({ id });
      } else if (type === 'read') {
        const { path } = data;
        const content = new TextDecoder().decode(Module.FS.readFile(path));
        self.postMessage({ id, result: content });
      } else if (type === 'compile') {
        const { path } = data;
        const result = compile(Module, path);
        self.postMessage({ id, result });
      }
    } catch (err: any) {
      console.error(err);
      self.postMessage({ id, error: err.toString() });
    }
  };
}

setup();
