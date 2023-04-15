#include "ThreadPool.h"

ThreadPool::ThreadPool() : done(false) {
    threads.emplace_back(&ThreadPool::work, this);
}

ThreadPool::ThreadPool(int num_thread) : done(false) {
    for (int i = 0; i < num_thread; i++) {
        threads.emplace_back(&ThreadPool::work, this);
    }
}

void ThreadPool::work() {
    while (!done) {
        std::function<void()> *task;
        if (task_queue.try_pop(task)) {
            (*task)();
        } else {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]() {
                return done || !task_queue.empty();
            });
        }
    }
}

ThreadPool::~ThreadPool() {
    done = true;
    cv.notify_all();
    for (std::thread &thread: threads) {
        thread.join();
    }
}

template<typename F, typename... Args>
auto ThreadPool::submit(F &&f, Args... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using result_type = typename std::result_of<F(Args...)>::type;
    auto packaged_task = std::make_shared<std::packaged_task<result_type()>>(
            std::packaged_task<result_type()>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            )
    );
    auto future = packaged_task->get_future();

    auto f_ptr = new std::function<void()>([packaged_task]() {
        (*packaged_task)();
    });

    task_queue.push(f_ptr);
    {
        std::unique_lock<std::mutex> lock(m);
        cv.notify_one();
    }
    return future;
}

template<typename F>
auto ThreadPool::submit(F &&f) -> std::future<typename std::result_of<F()>::type> {
    using result_type = typename std::result_of<F()>::type;
    auto packaged_task = std::make_shared<std::packaged_task<result_type()>>(
            std::packaged_task<result_type()>(std::forward<F>(f)));
    auto future = packaged_task->get_future();

    auto f_ptr = new std::function<void()>([packaged_task]() {
        (*packaged_task)();
    });


    task_queue.push(f_ptr);
    {
        std::unique_lock<std::mutex> lock(m);
        cv.notify_one();
    }
    return future;
}







