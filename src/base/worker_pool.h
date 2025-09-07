#ifndef BASE_WORKER_POOL_H_
#define BASE_WORKER_POOL_H_

#include <allegro5/allegro.h>
#include <functional>
#include <vector>
#include <deque>

// A generic worker pool that executes tasks concurrently.
//
// Note: if the code initiating the tasks needs all work to finish before it can continue, consider
// using std::for_each(std::execution::par_unseq, ...) instead (see zasm_for_every_script for an
// example).
class WorkerPool
{
public:
    // Creates the worker pool and starts the specified number of threads.
    explicit WorkerPool(size_t num_threads);

    // Destructor. Stops all threads and waits for them to finish.
    ~WorkerPool();

	size_t num_workers() const;

    void add_task(std::function<void()> task);

    void wait_for_all();

	// Drops all pending tasks and stops accepting new ones.
	void terminate();

private:
    // The main function for each worker thread.
    static void* worker_proc(ALLEGRO_THREAD* thread, void* arg);

    // The instance-specific loop for a worker thread.
    void worker_loop();

    std::vector<ALLEGRO_THREAD*> workers;
    std::deque<std::function<void()>> tasks;

    ALLEGRO_MUTEX* mutex;
    ALLEGRO_COND* task_cond;     // Signals workers that a new task is available.
    ALLEGRO_COND* finished_cond; // Signals when all tasks are done.

    size_t active_tasks = 0;
    bool stopping = false;
};

#endif
