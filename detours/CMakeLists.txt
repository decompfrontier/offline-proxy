add_library(detours STATIC
    detours.cpp modules.cpp disasm.cpp image.cpp creatwth.cpp disolx86.cpp disolx64.cpp disolia64.cpp disolarm.cpp disolarm64.cpp
)

target_compile_definitions(detours PRIVATE -D_WIN32_LEAN_AND_MEAN -D_UNICODE -D_WIN32_WINNT=0x601 PUBLIC -DDETOURS_DEBUG=0)
target_include_directories(detours PRIVATE ${CMAKE_CURRENT_LIST_DIR} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../)
