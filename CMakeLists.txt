cmake_minimum_required(VERSION 3.30)
project(proj_terminal)
set(CMAKE_CXX_STANDARD 20)
include_directories("C:\\Users\\USER\\CLionProjects\\proj_terminal\\eigen-3.4.0")
add_executable(proj_terminal main.cpp
        Component.h
        Circuit.h
        Circuit.cpp
        Component.cpp)
