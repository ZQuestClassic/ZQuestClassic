#ifndef ZC_ZASM_PIPELINE_H_
#define ZC_ZASM_PIPELINE_H_

#include "base/worker_pool.h"

void zasm_pipeline_init(bool force_precompile = false);
void zasm_pipeline_shutdown();

WorkerPool* zasm_pipeline_worker_pool();

#endif
