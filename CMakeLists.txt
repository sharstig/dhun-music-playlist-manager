cmake_minimum_required(VERSION 3.14)
project(DhunPlaylistManager VERSION 1.0.0 LANGUAGES CXX)

# Enforce C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include Directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find Threads library for multi-threaded HTTP server
find_package(Threads REQUIRED)

# Core DSA & JSON Library
add_library(dhun_core STATIC
    src/DoublyCircularList.cpp
    src/JsonHelper.cpp
)

# Compiler warnings
if(MSVC)
    target_compile_options(dhun_core PRIVATE /W4 /EHsc)
else()
    target_compile_options(dhun_core PRIVATE -Wall -Wextra -pedantic)
endif()

# Main REST API Server Executable
add_executable(dhun_server
    src/HttpServer.cpp
    src/main.cpp
)

target_link_libraries(dhun_server PRIVATE
    dhun_core
    Threads::Threads
)

if(WIN32)
    target_link_libraries(dhun_server PRIVATE ws2_32)
endif()

if(MSVC)
    target_compile_options(dhun_server PRIVATE /W4 /EHsc)
else()
    target_compile_options(dhun_server PRIVATE -Wall -Wextra -pedantic)
endif()

# DSA Unit Test Executable
enable_testing()
add_executable(dhun_tests
    tests/test_playlist.cpp
)

target_link_libraries(dhun_tests PRIVATE
    dhun_core
)

if(MSVC)
    target_compile_options(dhun_tests PRIVATE /W4 /EHsc)
else()
    target_compile_options(dhun_tests PRIVATE -Wall -Wextra -pedantic)
endif()

add_test(NAME DoublyCircularListUnitTests COMMAND dhun_tests)
