cmake_minimum_required(VERSION 3.19)
project(LongMath)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++20")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(bibl)
add_subdirectory(bibl)

add_executable(pi pi.cpp)
target_link_libraries(pi bibl)

add_executable(test tests.cpp)
target_link_libraries(test bibl)