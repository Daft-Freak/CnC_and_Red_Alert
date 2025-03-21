add_library(vqa32 STATIC
    config.cpp
    task.cpp
    loader.cpp
    drawer.cpp
    audio.cpp
    monodisp.cpp
    dstream.cpp
    #unvqbuff.asm
    #unvqvesa.asm
    vertag.cpp
    caption.cpp

    unvqbuff.cpp
    vqmstub.cpp
)

target_include_directories(vqa32 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../INCLUDE)
target_compile_definitions(vqa32 PRIVATE PORTABLE=1)
target_link_libraries(vqa32 PRIVATE port)

if(PICO_SDK_PATH)
    target_compile_definitions(vqa32 PRIVATE PICO_BUILD=1)
    target_link_libraries(vqa32 PRIVATE pico_stdlib)
else()
    find_package(SDL2 REQUIRED)
    target_link_libraries(vqa32 PRIVATE SDL2::SDL2)
endif()