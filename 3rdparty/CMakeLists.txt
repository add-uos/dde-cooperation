﻿
set(CMAKE_CXX_STANDARD 17)

# protobuf.a 需要加“-fPIC”， 否则无法连接到.so
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
add_subdirectory(protobuf)

# 检查coost目录是否存在 https://github.com/idealvin/coost 
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/coost")
    message(STATUS "coost目录不存在, 从GitHub下载")
    
    # 使用execute_process命令执行git clone命令来克隆代码
    execute_process(
        COMMAND git clone --depth 1 --branch v3.0.1 git@github.com:idealvin/coost.git
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
endif()

# coost 打开SSL和动态库
# cmake .. -DWITH_LIBCURL=ON -DWITH_OPENSSL=ON -DBUILD_SHARED_LIBS=ON
set(WITH_OPENSSL ON CACHE BOOL "build with openssl")
set(BUILD_SHARED_LIBS ON CACHE BOOL "build shared lib")
add_subdirectory(coost)

add_subdirectory(zrpc)