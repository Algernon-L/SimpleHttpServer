cmake_minimum_required(VERSION 2.6)
project(mymuduo)

# mymuduo最终编译成so动态库 设置动态库的路径 放置项目根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息  以及启动C++17语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(mymuduo/net)
add_subdirectory(mymuduo/utils)
add_subdirectory(mymuduo/http)

file(GLOB_RECURSE cpp_files 
${PROJECT_SOURCE_DIR}/mymuduo/net/*.cpp
${PROJECT_SOURCE_DIR}/mymuduo/http/*.cpp
${PROJECT_SOURCE_DIR}/mymuduo/utils/*.cpp)

# 编译动态库
add_library(mymuduo SHARED ${cpp_files})

# target_include_directories(mymuduo
#     PRIVATE
#         ${PROJECT_SOURCE_DIR})