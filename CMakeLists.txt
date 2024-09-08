cmake_minimum_required(VERSION 3.10)
project(webserver)

# 配置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -std=c++17")


# 配置头文件
include_directories(${PROJECT_SOURCE_DIR}/include/)

# 将指定目录下的源文件放入src_list中
aux_source_directory(./src SRC_LIST)

# # muduo 最终编译成so动态库， 设置动态库的路径
# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置可执行文件最终的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# # 编译为动态库
# add_library(mymuduo SHARED ${SRC_LIST})

# 生成可执行文件
add_executable(webserver ${SRC_LIST})

# 链接库
target_link_libraries(webserver pthread mysqlclient)

set(clientenv ./src1/webclient.cpp ./src/Logger.cpp ./src/Timestamp.cpp)
# set(clientenv ./src1/client.cpp)
add_executable(webclient ${clientenv})

target_link_libraries(webclient pthread)

# install(TARGETS echoserver client 
# RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin
# )



