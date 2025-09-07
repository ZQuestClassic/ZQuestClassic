#include "zc/zasm_pipeline.h"
#include "zc/jit.h"
#include "zc/zasm_optimize.h"
#include <thread>
#include <fmt/format.h>

static std::unique_ptr<WorkerPool> worker_pool;
static const int MAX_THREADS = 16;

template <typename... Args>
static void zasm_pipeline_trace(fmt::format_string<Args...> s, Args&&... args)
{
    std::string text = fmt::format(s, std::forward<Args>(args)...);
	al_trace("[zasm-pipeline] %s\n", text.c_str());
}

static int get_worker_thread_count()
{
	if (is_web())
		return 0;

	int num_threads = get_flag_int("-jit-threads").value_or(zc_get_config("ZSCRIPT", "jit_threads", -2));

	auto processor_count = std::thread::hardware_concurrency();
	if (num_threads < 0)
		num_threads = std::max(0, (int)processor_count / -num_threads);

	return std::min(MAX_THREADS, num_threads);
}

static void init_worker_pool(int num_threads)
{
	if (num_threads > 0)
	{
		worker_pool = std::make_unique<WorkerPool>(num_threads);
		zasm_pipeline_trace("created worker pool with {} threads", num_threads);
	}
}

enum class PipelineStage
{
	NotStarted,
	WaitingForOptimizer,
	Done,
};

static PipelineStage stage;

void zasm_pipeline_init(bool force_precompile)
{
	DCHECK(stage == PipelineStage::NotStarted);
	stage = PipelineStage::NotStarted;

	if (!jit_is_enabled() && !zasm_optimize_is_enabled())
	{
		zasm_pipeline_trace("zasm optimizer and jit are disabled");
		return;
	}

	int num_worker_threads = get_worker_thread_count();
	bool precompile = force_precompile || (jit_is_enabled() && is_feature_enabled("-jit-precompile", "ZSCRIPT", "jit_precompile", false));

	// Currently can only compile WASM on main browser thread, so always precompile scripts.
	if (is_web())
		precompile = true;
	if (get_flag_int("-test-bisect").has_value() || get_flag_int("-test-jit-bisect").has_value())
		precompile = true;
	if (!precompile && !num_worker_threads)
	{
		zasm_pipeline_trace("no threads assigned for worker pool, so must precompile");
		precompile = true;
	}

	if (precompile)
	{
		zasm_pipeline_trace("precompiling scripts. optimizer: {}, jit: {}", zasm_optimize_is_enabled(), jit_is_enabled());
		zasm_optimize_sync();
		init_worker_pool(num_worker_threads);
		if (jit_is_enabled())
			jit_startup(true);
		stage = PipelineStage::Done;
		return;
	}

	bool optimize_zasm_async = is_feature_enabled("-optimize-zasm-async", "ZSCRIPT", "optimize_zasm_async", false);
	if (optimize_zasm_async)
	{
		stage = PipelineStage::WaitingForOptimizer;
		init_worker_pool(num_worker_threads);
		zasm_optimize_async_init();
	}
	else
	{
		zasm_optimize_sync();
		init_worker_pool(num_worker_threads);
		stage = PipelineStage::Done;
	}
}

void zasm_pipeline_poll()
{
	DCHECK(stage != PipelineStage::NotStarted);

	switch (stage)
	{
		case PipelineStage::NotStarted:
		{
			return;
		}
		case PipelineStage::WaitingForOptimizer:
		{
			if (zasm_optimize_async_poll())
			{
				jit_startup(false);
				stage = PipelineStage::Done;
				if (jit_is_enabled())
					zasm_pipeline_trace("finished optimizing scripts, jit can now compile");
			}
			break;
		}
		case PipelineStage::Done:
		{
			return;
		}
	}
}

void zasm_pipeline_shutdown()
{
	jit_shutdown();

	if (worker_pool)
	{
		worker_pool->terminate();
		worker_pool.reset();
	}

	stage = PipelineStage::NotStarted;
}

WorkerPool* zasm_pipeline_worker_pool()
{
	return worker_pool.get();
}
