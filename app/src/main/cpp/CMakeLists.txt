cmake_minimum_required(VERSION 3.18.1)
project(native-lib)

add_library(native-lib SHARED native-lib.cpp)
find_library(log-lib log)

set(INJECT_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../inject)
add_library(inject SHARED IMPORTED)
set_target_properties(inject PROPERTIES
        IMPORTED_LOCATION ${INJECT_BASE}/build/intermediates/stripped_native_libs/debug/out/lib/${ANDROID_ABI}/libinject.so
        INTERFACE_INCLUDE_DIRECTORIES ${INJECT_BASE}/src/main/cpp/include)

include_directories(${INJECT_BASE}/src/main/cpp/include)

target_link_libraries(native-lib inject ${log-lib})