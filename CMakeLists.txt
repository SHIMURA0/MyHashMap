cmake_minimum_required(VERSION 4.0)
project(MyHashMap)

set(CMAKE_CXX_STANDARD 20)

add_executable(MyHashMap
        include/MyHashMap/MyHashMap.h
        src/MyHashMap.cpp
        examples/main.cpp)
