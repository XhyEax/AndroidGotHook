cmake_minimum_required(VERSION 3.18.1)
project(inject)

set(CMAKE_CXX_STANDARD 11)
include_directories(include/)
add_library(inject SHARED inject.cpp elfutil.cpp gotutil.cpp)
find_library(log-lib log)

target_link_libraries(inject ${log-lib})