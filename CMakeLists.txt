cmake_minimum_required(VERSION 3.7.1)

set(CMAKE_CXX_STANDARD 20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(app)

set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROCESSOR_ARCH "64")
else()
    set(PROCESSOR_ARCH "32")
endif()

if(WIN32)
    set(PLATFORM_NAME "win")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
elseif(APPLE)
    set(PLATFORM_NAME "osx")
endif()

add_executable(app main.cpp)

find_library(OPENVR_LIB openvr_api HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}" NO_DEFAULT_PATH)
target_link_libraries(app PUBLIC "${OPENVR_LIB}")

add_custom_command(
        TARGET app
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
        TARGET app
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/actions
        ${CMAKE_CURRENT_BINARY_DIR}
)