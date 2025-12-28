cmake_minimum_required(VERSION 3.10)
project(logs VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB TEST_FILES tests/*.cpp)

set(TEST_NAMES "")
foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    list(APPEND TEST_NAMES ${test_name})
endforeach()

set_target_properties(${TEST_NAMES} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
