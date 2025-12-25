cmake_minimum_required(VERSION 3.10)
project(logs VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加include目录到包含路径
include_directories(include)

# 收集所有头文件
file(GLOB HEADERS "include/*.hpp")

# 创建静态库
add_library(logs STATIC ${HEADERS})
set_target_properties(logs PROPERTIES LINKER_LANGUAGE CXX)

# 创建测试可执行文件
add_executable(test_log tests/test.cpp)
target_link_libraries(test_log logs pthread)

add_executable(testlog tests/testlog.cpp)
target_link_libraries(testlog logs pthread)

# 设置输出目录
set_target_properties(test_log testlog PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
