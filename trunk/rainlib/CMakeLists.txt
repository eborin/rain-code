cmake_minimum_required (VERSION 2.6)

file(GLOB headers *.h)
file(GLOB sources *.cpp)

include_directories ("${RAIn_SOURCE_DIR}/tracelib")

add_library(rainlib ${sources} ${headers})



