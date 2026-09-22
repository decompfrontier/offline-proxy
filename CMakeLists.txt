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

if (NOT DEFINED OFFLINEMOD_IP)
    set(OFFLINEMOD_IP "127.0.0.1")
endif()

if (NOT DEFINED OFFLINEMOD_PORT)
    set(OFFLINEMOD_PORT 9960)
endif()

if (NOT DEFINED OFFLINEMOD_DISABLE_HTTPS)
    set(OFFLINEMOD_DISABLE_HTTPS 1)
endif()

# 60 FPS render-loop cap. The shipped client ticks once per display vblank
# without internal gating, so on >60 Hz monitors animations play too fast
# and short clicks register as held-down. Set to 0 to disable the hook.
# Runtime override: set the OFFLINEMOD_FPS_CAP environment variable to change
# the cap without rebuilding (0 disables, 1..1000 sets the cap).
if (NOT DEFINED OFFLINEMOD_FPS_CAP)
    set(OFFLINEMOD_FPS_CAP 60)
endif()

# Compile in diagnostic logging for the FPS cap hook (writes %TEMP%\
# offlinemod_fps.log + OutputDebugString). Useful when debugging the hook
# attach state or verifying call rates; off in normal builds.
if (NOT DEFINED OFFLINEMOD_FPS_CAP_DIAG)
    set(OFFLINEMOD_FPS_CAP_DIAG 0)
endif()

add_subdirectory(detours)
add_subdirectory(OfflineMod.win32)
