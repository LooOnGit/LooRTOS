cmake_minimum_required(VERSION 3.20)
project(LooRTOS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(kernel)
add_subdirectory(arch)