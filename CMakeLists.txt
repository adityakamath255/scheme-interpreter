cmake_minimum_required(VERSION 3.16)
project(SchemeInterpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/interpreter
    ${PROJECT_SOURCE_DIR}/include/builtins
)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(scheme ${SOURCES})

target_compile_options(scheme PRIVATE -O3 -Wall)

add_subdirectory(third_party/replxx)
target_link_libraries(scheme PRIVATE replxx)
