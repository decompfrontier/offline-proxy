configure_file("${CMAKE_CURRENT_LIST_DIR}/serverconfig.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/generated/serverconfig.h")

add_library(offlineproxy SHARED
    main.cpp
    curl_exp.cpp
    fps_cap.cpp
    fps_cap.h
    Source.def
    #Resource.rc
    resource.h
    pch.h
    "${CMAKE_CURRENT_BINARY_DIR}/generated/serverconfig.h"
)

target_precompile_headers(offlineproxy PRIVATE pch.cpp)
target_link_libraries(offlineproxy PRIVATE
    detours
    CURL::libcurl
    wininet
    winhttp
)
set_target_properties(offlineproxy PROPERTIES
    OUTPUT_NAME ${CURL_NAME}
    OUTPUT_NAME_Debug ${CURL_NAME}
    OUTPUT_NAME_RelWithDebInfo ${CURL_NAME}
)

target_compile_definitions(offlineproxy PRIVATE -D_UNICODE -D_WIN32_WINNT=0x601)
target_include_directories(offlineproxy PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
