CMAKE_MINIMUM_REQUIRED (VERSION 2.8.10)
project(ClientProject)



INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/gio-unix-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include/
)

add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/Sources/main.cpp)

target_link_libraries(client /home/tang/project/dbus_exercise_tangshengjia/ServiceProject/build/libtraining.so.1.0.0 pthread gio-2.0 gobject-2.0 gmodule-2.0 gthread-2.0 rt glib-2.0)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(client ${OPENSSL_LIBRARIES})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(client PRIVATE -g)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(client PRIVATE -O2 -s)
endif()
