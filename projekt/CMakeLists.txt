cmake_minimum_required(VERSION 3.22)
project(P2P)

set(CMAKE_CXX_STANDARD 20)

add_executable(P2P src/main.cpp
        src/ResourceManager.cpp
        src/ResourceManager.h
)