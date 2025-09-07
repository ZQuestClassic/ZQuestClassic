#include "base/worker_pool.h"

WorkerPool::WorkerPool(size_t num_threads)
{
	mutex = al_create_mutex();
	task_cond = al_create_cond();
	finished_cond = al_create_cond();

	for (size_t i = 0; i < num_threads; i++)
	{
		ALLEGRO_THREAD* thread = al_create_thread(worker_proc, this);
		workers.push_back(thread);
		al_start_thread(thread);
	}
}

WorkerPool::~WorkerPool()
{
	al_lock_mutex(mutex);
	stopping = true;
	al_broadcast_cond(task_cond); // Wake up all threads to exit
	al_unlock_mutex(mutex);

	for (ALLEGRO_THREAD* worker : workers)
	{
		al_join_thread(worker, nullptr);
		al_destroy_thread(worker);
	}

	al_destroy_mutex(mutex);
	al_destroy_cond(task_cond);
	al_destroy_cond(finished_cond);
}

size_t WorkerPool::num_workers() const
{
	return workers.size();
}

void WorkerPool::add_task(std::function<void()> task)
{
	al_lock_mutex(mutex);
	tasks.push_back(std::move(task));
	active_tasks++;
	al_signal_cond(task_cond); // Signal one worker
	al_unlock_mutex(mutex);
}

void WorkerPool::wait_for_all()
{
	al_lock_mutex(mutex);
	while (active_tasks > 0)
	{
		al_wait_cond(finished_cond, mutex);
	}
	al_unlock_mutex(mutex);
}

void WorkerPool::terminate()
{
	al_lock_mutex(mutex);
	stopping = true;
	tasks.clear();
	al_unlock_mutex(mutex);
}

// Static function passed to al_create_thread.
void* WorkerPool::worker_proc(ALLEGRO_THREAD* thread, void* arg)
{
	WorkerPool* pool = static_cast<WorkerPool*>(arg);
	pool->worker_loop();
	return nullptr;
}

void WorkerPool::worker_loop()
{
	while (true)
	{
		std::function<void()> task;

		al_lock_mutex(mutex);

		// Wait until there's a task or we're stopping.
		while (!stopping && tasks.empty())
		{
			al_wait_cond(task_cond, mutex);
		}

		if (stopping && tasks.empty())
		{
			al_unlock_mutex(mutex);
			return;
		}

		// Get the next task.
		task = std::move(tasks.front());
		tasks.pop_front();

		al_unlock_mutex(mutex);

		// Execute the task.
		if (task)
			task();

		// Decrement active task count and notify waiters if done.
		al_lock_mutex(mutex);
		active_tasks--;
		if (active_tasks == 0)
		{
			al_broadcast_cond(finished_cond);
		}
		al_unlock_mutex(mutex);
	}
}
