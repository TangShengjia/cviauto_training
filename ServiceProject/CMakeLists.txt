CMAKE_MINIMUM_REQUIRED (VERSION 2.8.10)
project(ServiceProject)

EXECUTE_PROCESS(COMMAND gdbus-codegen --generate-c-code ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/testservice --c-namespace TestService ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/org.example.ITestService.xml
                COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/testservice.c ${CMAKE_CURRENT_SOURCE_DIR}/Sources
                COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/testservice.h ${CMAKE_CURRENT_SOURCE_DIR}/Include
                COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/testservice.h ${CMAKE_CURRENT_SOURCE_DIR}/../ClientProject/Include
                COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/testservice.h
                COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/GDBus/testservice.c
                )


INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/gio-unix-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include/
)

add_executable(server 
               ${CMAKE_CURRENT_SOURCE_DIR}/Sources/main.cpp 
               ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TestInfo.cpp 
               ${CMAKE_CURRENT_SOURCE_DIR}/Sources/testservice.c)

target_link_libraries(server  pthread gio-2.0 gobject-2.0 gmodule-2.0 gthread-2.0 rt glib-2.0)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(server ${OPENSSL_LIBRARIES})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(server PRIVATE -g)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(server PRIVATE -O2 -s)
endif()


add_library(training SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TestInfo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/Sources/testservice.c
)

set_target_properties(training PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

target_link_libraries(training
    pthread
    gio-2.0
    gobject-2.0
    gmodule-2.0
    gthread-2.0
    rt
    glib-2.0
)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(training ${OPENSSL_LIBRARIES})
endif()