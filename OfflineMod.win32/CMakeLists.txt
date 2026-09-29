configure_file("${CMAKE_CURRENT_LIST_DIR}/serverconfig.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/generated/serverconfig.h")

add_library(offlineproxy SHARED
    main.cpp
    libcurl_export.cpp
    Source.def
    #Resource.rc
    resource.h
    pch.h
    console.h
    config.h
    config.cpp
    log.h
    log.cpp
    util.h
    util.cpp
    patch.h
    patch/patch_http.cpp
    patch/patch_fps.cpp
    patch/patch_http.h
    patch/patch_fps.h
    proxyep.h
)

target_precompile_headers(offlineproxy PRIVATE pch.h)
target_link_libraries(offlineproxy PRIVATE
    wininet
    winhttp
    detours    
    CURL::libcurl
    unofficial::inih::libinih
    unofficial::inih::inireader
)
set_target_properties(offlineproxy PROPERTIES
    OUTPUT_NAME ${CURL_NAME}
    OUTPUT_NAME_Debug ${CURL_NAME}
    OUTPUT_NAME_RelWithDebInfo ${CURL_NAME}
)

target_compile_definitions(offlineproxy PRIVATE -D_UNICODE -D_WIN32_WINNT=0x602) # Windows 8.0+
target_include_directories(offlineproxy PRIVATE ${CMAKE_CURRENT_LIST_DIR})
