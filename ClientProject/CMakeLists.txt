cmake_minimum_required(VERSION 2.8.10)
project(ClientProject)

include_directories(
    ../Common/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

link_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(client
    Sources/main.cpp
    Sources/testservice.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)

target_include_directories(client PRIVATE
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
)
target_link_libraries(client
    ${GLIB2_LIBRARIES} ${GIO2_LIBRARIES}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(client PRIVATE -g)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(client PRIVATE -O2 -s)
endif()
