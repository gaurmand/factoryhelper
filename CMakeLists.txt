cmake_minimum_required(VERSION 3.16)
project(factoryhelper)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

include_directories("include")

add_subdirectory(src)
add_subdirectory(tests)
