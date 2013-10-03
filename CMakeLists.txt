cmake_minimum_required (VERSION 2.6)

project (RAIn)

# Add libraries
add_subdirectory(arglib)
add_subdirectory(tracelib)
add_subdirectory(rainlib)

# Add executable
add_executable(rain_tool.bin main.cpp)

include_directories ("${PROJECT_SOURCE_DIR}/arglib")
include_directories ("${PROJECT_SOURCE_DIR}/tracelib")
include_directories ("${PROJECT_SOURCE_DIR}/rainlib")

target_link_libraries (rain_tool.bin arglib tracelib rainlib)

