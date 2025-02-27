add_library(offlineproxy SHARED
    main.cpp
    curl_exp.cpp
    Source.def
    #Resource.rc # dio porco
    resource.h
    pch.h
)

target_precompile_headers(offlineproxy PRIVATE pch.cpp)
target_link_libraries(offlineproxy PRIVATE
    detours
    CURL::libcurl
    wininet
    winhttp
)
set_target_properties(offlineproxy PROPERTIES
    OUTPUT_NAME curl
    OUTPUT_NAME_Debug curl
    OUTPUT_NAME_RelWithDebInfo curl
)

target_compile_definitions(offlineproxy PRIVATE -D_UNICODE -D_WIN32_WINNT=0x601)

