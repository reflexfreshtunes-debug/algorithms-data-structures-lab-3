cmake_minimum_required(VERSION 3.10)
project(asd_lab_3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(task_01 task_01/main.cpp)
add_executable(task_02 task_02/main.cpp)
target_include_directories(task_01 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(task_02 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

