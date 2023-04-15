#include "ThreadPool.h"
#include <iostream>
#include <thread>
#include <chrono>



int task(){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 1;
}

int main() {
    ThreadPool tp(16);

    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < 100; ++i) {
        tp.submit(task);
    }

    auto end = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<"time :"<< time.count() << std::endl;

}