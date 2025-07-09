cmake_minimum_required(VERSION 2.8.10)
project(ServerProject)

include_directories(
    ../Common/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

link_directories(${CMAKE_CURRENT_BINARY_DIR})


add_executable(server
    Sources/main.cpp
    Sources/testservice.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)

target_include_directories(server PRIVATE
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
)
target_link_libraries(server
    ${GLIB2_LIBRARIES} ${GIO2_LIBRARIES}
)
target_link_libraries(server pthread)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(server PRIVATE -g)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(server PRIVATE -O2 -s)
endif()
