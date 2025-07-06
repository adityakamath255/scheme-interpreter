cmake_minimum_required(VERSION 3.16)
project(SchemeInterpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(WARNING "Git submodules could not be updated. Build may fail.")
    endif()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/interpreter
    ${PROJECT_SOURCE_DIR}/include/builtins
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(scheme ${SOURCES})

target_compile_options(scheme PRIVATE -O3)

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/replxx/CMakeLists.txt")
    add_subdirectory(third_party/replxx)
    target_link_libraries(scheme PRIVATE replxx)
else()
    message(FATAL_ERROR "replxx dependency missing. Run 'git submodule update --init' or manually install replxx in third_party/")
endif()
