cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(qm2c)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS filesystem)
include_directories(${Boost_INCLUDE_DIRS})

find_package(LLVM REQUIRED CONFIG)

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist" CACHE STRING "Install path" FORCE)
set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/build" CACHE STRING "Build directory" FORCE)

add_subdirectory(./src)
