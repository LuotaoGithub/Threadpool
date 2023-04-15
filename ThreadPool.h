#pragma ones

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <future>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex m;
public:
    ~ThreadSafeQueue() = default;

    bool try_pop(T &v) {
        std::lock_guard<std::mutex> lk(m);
        if (queue.empty()) {
            return false;
        } else {
            v = std::move(queue.front());
            queue.pop();
            return true;
        }
    }

    bool push(const T &v) {
        std::lock_guard<std::mutex> lk(m);
        queue.push(v);
        return true;
    }


    bool empty() {
        std::lock_guard<std::mutex> lk(m);
        return queue.empty();
    }
};


class ThreadPool {
public:
    //线程池是否停止
    std::atomic_bool done;
private:
    //锁
    std::mutex m;
    std::condition_variable cv;

    //任务队列,类型为函数指针
    ThreadSafeQueue<std::function<void()> *> task_queue;
    //线程数组
    std::vector<std::thread> threads;

private:
    void work() {
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

public:
    explicit ThreadPool() : done(false) {
        threads.emplace_back(&ThreadPool::work, this);
    }

    explicit ThreadPool(int num_thread) : done(false) {
        for (int i = 0; i < num_thread; i++) {
            threads.emplace_back(&ThreadPool::work, this);
        }
    }

    ~ThreadPool() {
        done = true;
        cv.notify_all();
        for (std::thread &thread: threads) {
            thread.join();
        }
    }

    //提交一个不带参数的function,返回一个futhre
    template<typename F>
    std::future<typename std::result_of<F()>::type> submit(F &&f) {
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

    //提交一个带参数的function,返回一个future
    template<typename F, typename ...Args>
    std::future<typename std::result_of<F(Args...)>::type> submit(F &&f, Args ...args) {
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

};