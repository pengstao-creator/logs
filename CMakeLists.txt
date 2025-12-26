cmake_minimum_required(VERSION 3.10)
project(logs VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB HEADERS "include/*.hpp")
file(GLOB SOURCES "src/*.cpp")

add_library(mylog SHARED ${SOURCES})
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
