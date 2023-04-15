cmake_minimum_required(VERSION 3.16)
project(threadpool)

set(CMAKE_CXX_STANDARD 17)

add_executable(threadpool
        main.cpp
        ThreadPool.h
        ThreadPool.cpp
        )
target_link_libraries(threadpool pthread)