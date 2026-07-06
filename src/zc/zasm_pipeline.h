#ifndef ZC_ZASM_PIPELINE_H_
#define ZC_ZASM_PIPELINE_H_

#include "components/worker_pool/worker_pool.h"

void zasm_pipeline_init(bool force_precompile = false);
void zasm_pipeline_shutdown();

WorkerPool* zasm_pipeline_worker_pool();

// The -jit-threads / [ZSCRIPT] jit_threads setting, resolved: 0 means
// synchronous compilation, a negative value divides the machine's core count
// (e.g. -2 = half the cores), floored at min_threads. The single owner of
// these semantics - callers (the engine worker pool, the wasm backend's own
// precompile threading) only apply their own caps.
int jit_requested_threads(int min_threads);

#endif
