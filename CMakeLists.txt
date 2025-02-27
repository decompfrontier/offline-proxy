cmake_minimum_required(VERSION 3.20)
project(offlineproxy)

if (NOT WIN32)
    message(FATAL_ERROR "The offline proxy is supported only in Windows")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/bin")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static" CACHE STRING "" FORCE)
    set(CURL_NAME curl)
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP")
    set(CURL_NAME libcurl)
endif()

if (MINGW)
    add_definitions(-D__MINGW_BUILD__)
endif()

if (OFFLINEMOD_DEPLOY)
    add_definitions(-DOFFLINE_DEPLOY)
endif()

find_package(CURL REQUIRED)

add_subdirectory(detours)
add_subdirectory(OfflineMod.win32)
