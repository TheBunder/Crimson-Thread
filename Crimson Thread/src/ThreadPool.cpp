#include "include/ThreadPool.h"
#include "include/Visualizer.h"

ThreadPool::ThreadPool(unsigned int num_threads)
{
    if (num_threads > std::thread::hardware_concurrency() + 3) {
        PrintWarning("Warning: constructed ThreadPoll with higher thread amount then CPU core.");
    }

    // Creat all the recuested threads
    for (int i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);

                    // Waiting for task or for distractor
                    cv_.wait(lock, [this] {
                        return !tasks_.empty() || stop_;
                    });

                    // stop the loop if there are no tasks and the pool is being distracted
                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    // Get the next task from the queue
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                // Other threads will be abel to use the queue will task() is running
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    // Wake up all threads
    cv_.notify_all();

    // Joining all threads to make sure the system waits for them all
    for (auto& thread : threads_) {
        thread.join();
    }
}

void ThreadPool::Enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        // Increment active tasks count
        active_tasks_++;

        // Wrap the task so calling it will also update the queue
        auto wrapped_task = [this, task = std::move(task)]() {
            // Call task
            task();

            // Decrement active tasks and notify if all tasks are done
            int remaining;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                remaining = --active_tasks_;
            }

            // If this was the last task, notify
            if (remaining == 0) {
                tasks_done_cv_.notify_all();
            }
        };

        tasks_.emplace(std::move(wrapped_task));
    }
    cv_.notify_one();
}

void ThreadPool::WaitAll() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    tasks_done_cv_.wait(lock, [this]() {
        return active_tasks_ == 0 && tasks_.empty();
    });
}