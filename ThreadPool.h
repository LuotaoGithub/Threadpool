
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
    void work();

public:
    explicit ThreadPool();

    explicit ThreadPool(int num_thread);

    ~ThreadPool();

    //提交一个不带参数的function,返回一个futhre
    template<typename F>
    std::future<typename std::result_of<F()>::type> submit(F &&f);

    //提交一个带参数的function,返回一个future
    template<typename F, typename ...Args>
    auto submit(F &&f, Args ...args) -> std::future<typename std::result_of<F(Args...)>::type>;

};