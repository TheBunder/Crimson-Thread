#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <atomic>

// Class that represents a simple thread pool
class ThreadPool {
public:
    // Constructor to create a thread pool with given number of threads
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency());

    // Destructor to stop the thread pool
    ~ThreadPool();

    // Enqueue task for execution by the thread pool
    void enqueue(std::function<void()> task);

    // Wait for all the thread to finish running
    void wait_all();
private:
    // Vector to store worker threads
    std::vector<std::thread> threads_;

    // Queue of tasks
    std::queue<std::function<void()>> tasks_;

    // Mutex to synchronize access to shared data
    std::mutex queue_mutex_;

    // Condition variable to signal changes in the state of the tasks queue
    std::condition_variable cv_;

    // Additional condition variable for waiting on task completion
    std::condition_variable tasks_done_cv_;

    // Counter for active tasks
    std::atomic<int> active_tasks_{0};

    // Flag to indicate whether the thread pool should stop or not
    bool stop_ = false;
};

#endif //THREADPOOL_H
