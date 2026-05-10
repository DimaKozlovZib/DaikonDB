cmake_minimum_required(VERSION 3.15)
project(daikon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES 
    lib/src/main.cpp
    lib/src/CommandParser.cpp
    lib/src/Tokenizer.cpp
)

add_executable(daikon_app ${SOURCES})

add_library(daikon_lib INTERFACE)
target_include_directories(daikon_lib INTERFACE include/)

target_link_libraries(daikon_app PRIVATE daikon_lib)

target_include_directories(daikon_app PRIVATE include)