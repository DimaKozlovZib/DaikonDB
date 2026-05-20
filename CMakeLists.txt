cmake_minimum_required(VERSION 3.15)
project(daikon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(LIB_SOURCES
    lib/src/Database/DaikonDB_Common.cpp
    lib/src/Database/DaikonDB_Geo.cpp
    lib/src/Database/DaikonDB_List.cpp
    lib/src/Database/DaikonDB_Set.cpp
    lib/src/Database/DaikonDB_String.cpp
    lib/src/Parsing/CommandParser.cpp
    lib/src/Parsing/Tokenizer.cpp
)

add_library(daikon_lib STATIC ${LIB_SOURCES})
target_include_directories(daikon_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/lib/includes
)

set(APP_SOURCES
    app/main.cpp
)

add_executable(daikon_app ${APP_SOURCES})
target_link_libraries(daikon_app PRIVATE daikon_lib)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
    Tests/test_types.cpp
    Tests/test_memory.cpp
    Tests/test_daikondb.cpp
    Tests/test_matchpat.cpp
    Tests/test_tokens.cpp
)

add_executable(daikon_tests ${TEST_SOURCES})
target_link_libraries(daikon_tests PRIVATE daikon_lib gtest gtest_main)
target_include_directories(daikon_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/includes)
include(GoogleTest)
gtest_discover_tests(daikon_tests)

include(Tests/coverage.cmake)