add_library(picolib STATIC
    audio.cpp
    drawbuff_pico.cpp
    file.cpp
    keyboard.cpp
    lwip.c
    mouse.cpp
    timer.cpp
    window.cpp

    # a lot of SDLLIB isn't SDL specific (or is a stub)
    ../SDLLIB/buffer.cpp
    ../SDLLIB/dipthong.cpp
    ../SDLLIB/drawbuff.cpp
    ../SDLLIB/font.cpp
    ../SDLLIB/iconcach.cpp
    ../SDLLIB/iff.cpp
    ../SDLLIB/mem.cpp
    ../SDLLIB/misc.cpp
    ../SDLLIB/net_select.cpp
    ../SDLLIB/palette.cpp
    ../SDLLIB/playcd.cpp
    ../SDLLIB/shape.cpp
    ../SDLLIB/wincomm.cpp
    ../SDLLIB/wsa.cpp
)

target_include_directories(picolib PUBLIC ../SDLLIB/include) # hax
target_include_directories(picolib PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# lwip usage is mostly so socket code compiles
target_link_libraries(picolib PUBLIC pico_stdlib pico_lwip pico_lwip_arch)
target_include_directories(picolib PUBLIC ${PICO_LWIP_PATH}/src/include/compat/posix)