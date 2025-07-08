cmake_minimum_required(VERSION 3.10)
project(TestServiceClient)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)

# 查找依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)

# 添加头文件目录
include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${GIO_UNIX_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/../include
)

# 添加库目录
link_directories(
    ${GLIB_LIBRARY_DIRS}
    ${GIO_LIBRARY_DIRS}
    ${GIO_UNIX_LIBRARY_DIRS}
)

# 添加源文件
set(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/../sources/Client/Clientlmpl.cpp
)

# 添加可执行文件
add_executable(client ${SOURCE_FILES})

# 链接库
target_link_libraries(client
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
    pthread
    rt
)

# 设置编译选项
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s")

# 安装目标
install(TARGETS client DESTINATION bin)    