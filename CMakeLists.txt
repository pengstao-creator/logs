cmake_minimum_required(VERSION 3.10)
project(logs VERSION 2.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include_directories(include)

file(GLOB HEADERS "include/*.hpp")
file(GLOB SOURCES "src/*.cpp")

add_library(mylog SHARED ${SOURCES})

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(mylog PRIVATE include/pch.hpp)
endif()

set_target_properties(mylog PROPERTIES LINKER_LANGUAGE CXX)

add_executable(test_log tests/test.cpp)
target_link_libraries(test_log mylog pthread)

add_executable(testlog tests/testlog.cpp)
target_link_libraries(testlog mylog pthread)

set_target_properties(mylog PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

set_target_properties(test_log testlog PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
