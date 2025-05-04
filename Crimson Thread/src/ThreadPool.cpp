#include "include/ThreadPool.h"

ThreadPool::ThreadPool(unsigned int num_threads)
{
    // Creating worker threads
    for (int i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                // The reason for putting the below code here is to unlock the queue
                // before executing the task so that other threads can perform enqueue tasks
                {
                    // Locking the queue so that data can be shared safely
                    std::unique_lock<std::mutex> lock(queue_mutex_);

                    // Waiting until there is a task to execute or the pool is stopped
                    cv_.wait(lock, [this] {
                        return !tasks_.empty() || stop_;
                    });

                    // exit the thread in case the pool is stopped and there are no tasks
                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    // Get the next task from the queue
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        // Lock the queue to update the stop flag safely
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    // Notify all threads
    cv_.notify_all();

    // Joining all worker threads to ensure they have completed their tasks
    for (auto& thread : threads_) {
        thread.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        // Increment active tasks count
        active_tasks_++;

        // Wrap the task to track completion
        auto wrapped_task = [this, task = std::move(task)]() {
            // Execute the original task
            task();

            // Decrement active tasks and notify if all tasks are done
            int remaining;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                remaining = --active_tasks_;
            }

            // If this was the last task, notify anyone waiting
            if (remaining == 0) {
                tasks_done_cv_.notify_all();
            }
        };

        tasks_.emplace(std::move(wrapped_task));
    }
    cv_.notify_one();
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    tasks_done_cv_.wait(lock, [this]() {
        return active_tasks_ == 0 && tasks_.empty();
    });
}