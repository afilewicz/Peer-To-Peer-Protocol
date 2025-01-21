cmake_minimum_required(VERSION 3.22)
project(P2P)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(P2P src/main.cpp
        src/ResourceManager.cpp
        src/ResourceManager.h
        src/UDPCommunicator.cpp
        src/UDPCommunicator.h
)