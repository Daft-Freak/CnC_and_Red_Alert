add_library(sdllib STATIC
    audio.cpp
    buffer.cpp
    dipthong.cpp
    drawbuff.cpp
    drawbuff_sdl.cpp
    file.cpp
    font.cpp
    iconcach.cpp
    iff.cpp
    keyboard.cpp
    mem.cpp
    misc.cpp
    mouse.cpp
    net_select.cpp
    palette.cpp
    playcd.cpp
    shape.cpp
    timer.cpp
    wincomm.cpp
    window.cpp
    wsa.cpp
)
target_include_directories(sdllib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

# somewhere in the SDL headers, this ends up enabling __WIN32__ and then it tries to include windows headers
target_compile_options(sdllib PRIVATE -UWIN32)


find_package(SDL2 REQUIRED)
target_link_libraries(sdllib SDL2::SDL2)

# optional serial port support
find_library(LIBSERIALPORT_LIBRARIES serialport)
find_path(LIBSERIALPORT_INCLUDE_DIR libserialport.h)
if (LIBSERIALPORT_LIBRARIES AND LIBSERIALPORT_INCLUDE_DIR)
    message("Found libserialport")
    message("[MODEM NOISES]")

    target_compile_definitions(sdllib PRIVATE LIBSERIALPORT)
    target_include_directories(sdllib PRIVATE ${LIBSERIALPORT_INCLUDE_DIR})
    target_link_libraries(sdllib ${LIBSERIALPORT_LIBRARIES})
endif()