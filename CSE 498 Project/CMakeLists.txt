cmake_minimum_required(VERSION 4.1)
project(CSE_498_Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(CSE_498_Project
        character.h
        character.cpp
        main.cpp
        item.h
        item.cpp
        dungeonData.h
        dungeonData.cpp)
