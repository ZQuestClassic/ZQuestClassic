/** @type {Map<number, WebAssembly.Instance>} */
const wasmInstances = new Map();
/** @type {Map<number, string>} */
const wasmModuleNames = new Map();
let nextWasmModuleId = 1;

/** @type {Map<number, {id: number, moduleInstance: WebAssembly.Instance, name: string}>} */
const scriptHandles = new Map();

const emFunctions = {
  doCommands: null,
  getRegister: null,
  setRegister: null,
  runtimeDebug: null,
  logError: null,
};

let nextHandleId = 1;
function createScriptHandle(moduleId) {
  const id = nextHandleId++;
  const name = `${id} - ${wasmModuleNames.get(moduleId)}`;
  const instance = wasmInstances.get(moduleId);
  const scriptHandle = { id, instance, name };
  scriptHandles.set(id, scriptHandle);
  return scriptHandle;
}

function assert(value, message) {
  if (!value) {
    throw new Error(message);
  }
}

function assertMainThread() {
  assert(typeof window !== 'undefined', 'expected to be called from main thread');
}

// TODO: maybe need? for compile on workers?
function waitForNextWorkerMessage(worker) {
  return new Promise((resolve, reject) => {
    worker.addEventListener('message', e => {
      resolve(e.data);
    });
    worker.addEventListener('error', reject);
  });
}

let currentReturnValue;
function onSetReturnValue(value) {
  currentReturnValue = value;
}

// Returns the engine's raw wasm export for an exported C function, so it can be
// bound as a script-module import and called wasm->wasm with no JS frame. These
// imports are on the script hot path (get/set_register per engine-register
// access, do_commands per uncompiled-op batch), and going through JS was a
// dominant cost for engine-heavy scripts.
//
// With ASYNCIFY, `Module._name` is not the raw function: the glue wraps every
// wasm export in a JS bookkeeping wrapper (Asyncify.instrumentFunction), and the
// raw exports are only reachable under minified names. But the glue records
// original -> wrapper in Asyncify.funcWrappers, so invert that to recover the
// raw function. Bypassing the wrapper is safe for these imports: script wasm
// only executes while Asyncify is in the Normal state, and the sync engine
// callbacks never unwind (RUNGENFRZSCR - the one command that suspends the
// script - yields back to run_script instead; see jit_wasm.cpp).
function rawEngineExport(name) {
  const wrapped = Module['_' + name];
  assert(wrapped, `no such engine export: ${name}`);
  const asyncify = globalThis.Asyncify;
  if (asyncify?.funcWrappers) {
    for (const [original, wrapper] of asyncify.funcWrappers) {
      if (wrapper === wrapped) return original;
    }
  }
  // Emscripten internals moved - the instrumented export still works (and still
  // skips cwrap's per-call argument marshaling), it just pays a JS frame.
  console.warn(`could not resolve raw wasm export for ${name}; script->engine calls will go through JS`);
  return wrapped;
}

export async function compileScriptWasmModule(name, ptr, size) {
  // TODO  support compiling in workers.
  assertMainThread();

  if (!emFunctions.doCommands) {
    emFunctions.doCommands = rawEngineExport('em_do_commands');
    // Some commands may temporarily control the game loop (ex: RUNGENFRZSCR), so they need to be done async.
    emFunctions.doCommandsAsync = Module.cwrap('em_do_commands', 'int', ['int', 'int', 'int'], {async: true});
    emFunctions.getRegister = rawEngineExport('em_get_register');
    emFunctions.setRegister = rawEngineExport('em_set_register');
    emFunctions.setGuardedRegister = rawEngineExport('em_set_guarded_register');
    emFunctions.runtimeDebug = rawEngineExport('em_runtime_script_debug');
    emFunctions.logError = rawEngineExport('em_log_error');
    emFunctions.podRead = rawEngineExport('em_pod_read');
    emFunctions.podWrite = rawEngineExport('em_pod_write');
    emFunctions.allocatemem = rawEngineExport('em_allocatemem');
    emFunctions.writepodarr = rawEngineExport('em_writepodarr');
  }

  console.log(`compiling ${name}, ${size} len`);
  try {
    const buffer = new Uint8Array(Module.HEAPU8.buffer, ptr, size);
    const module = await WebAssembly.compile(buffer);
    console.log(`compiling ${name} DONE`);
    const id = nextWasmModuleId++;

    const instance = await WebAssembly.instantiate(module, {
      env: {
        memory: Module.wasmMemory,
        set_return_value: onSetReturnValue,
        do_commands: emFunctions.doCommands,
        do_commands_async: emFunctions.doCommandsAsync,
        get_register: emFunctions.getRegister,
        set_register: emFunctions.setRegister,
        set_guarded_register: emFunctions.setGuardedRegister,
        runtime_debug: emFunctions.runtimeDebug,
        log_error: emFunctions.logError,
        pod_read: emFunctions.podRead,
        pod_write: emFunctions.podWrite,
        allocatemem: emFunctions.allocatemem,
        writepodarr: emFunctions.writepodarr,
      },
    });

    wasmInstances.set(id, instance);
    wasmModuleNames.set(id, name);
    return id;
  } catch (err) {
    console.error(`error compiling ${name}`, err);
    return 0;
  }
}

export function createScriptWasmHandle(moduleId) {
  assertMainThread();

  const instance = wasmInstances.get(moduleId);
  if (!instance) return 0;

  const { id } = createScriptHandle(moduleId);
  return id;
}

// Runs the wasm module in a worker.
// Only need one worker, as only one ZASM script runs at any time.
// When one does run, it necessarily blocks the application main thread.
export async function pollScriptWasmHandle(handleId, ptr) {
  assertMainThread();

  const scriptHandle = scriptHandles.get(handleId);
  if (!scriptHandle) return false;

  const a = Module.HEAP32[ptr/4];
  const b = Module.HEAP32[ptr/4+1];
  const c = Module.HEAP32[ptr/4+2];
  const d = Module.HEAP32[ptr/4+3];
  const e = Module.HEAP32[ptr/4+4];
  const f = Module.HEAP32[ptr/4+5];
  const g = Module.HEAP32[ptr/4+6];
  const h = Module.HEAP32[ptr/4+7];
  const args = [a, b, c, d, e, f, g, h];
  scriptHandle.args = args;

  const RUNSCRIPT_OK = 0;
  const RUNSCRIPT_ERROR = 1;
  
  currentReturnValue = -1;
  try {
    // This is expected to throw if the script execution must stop.
    // This can be for expected reasons (the script was deleted via RUNSCRIPT_SELFDELETE),
    // or for fatal errors. To mark which, before the script stops it calls set_return_value.
    scriptHandle.instance.exports.run(...args);
    currentReturnValue = RUNSCRIPT_OK;
  } catch (err) {
    if (currentReturnValue === -1) currentReturnValue = RUNSCRIPT_ERROR;
    if (currentReturnValue == RUNSCRIPT_ERROR) {
      console.error(err);
      return currentReturnValue;
    }
  }

  return currentReturnValue;
}

export async function destroyScriptWasmHandle(handleId) {
  assertMainThread();

  const scriptHandle = scriptHandles.get(handleId);
  if (!scriptHandle) return false;

  scriptHandles.delete(handleId);
  return true;
}

export async function destroyAllScriptWasmHandles() {
  for (const handleId of scriptHandles.keys()) {
    await destroyScriptWasmHandle(handleId);
  }
}
