import Module from './zscript.data.js';
import ZScript from './zscript.mjs';

async function createModule() {
  Module.noInitialRun = true;
  await ZScript(Module);
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
      } else if (type === 'write') {
        const { path, code } = data;
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
      self.postMessage({ id, error: err.toString() });
    }
  };
}

setup();
