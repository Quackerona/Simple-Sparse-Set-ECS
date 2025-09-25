cmake_minimum_required(VERSION 3.15)

project(SimpleSparseSetECS LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    SimpleExample.cpp
)

add_executable(SimpleSparseSetECSExample ${SOURCES})