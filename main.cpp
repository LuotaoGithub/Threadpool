#include "th.h"
#include <iostream>
#include <thread>
#include <chrono>



int task(int n){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return n;
}

int main() {
    ThreadPool tp(1);

    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < 100; ++i) {
        auto f = tp.submit(task,0);
        
    }

    auto end = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<"time :"<< time.count() << std::endl;

}