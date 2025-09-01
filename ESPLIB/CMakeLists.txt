add_library(esplib STATIC
    audio.cpp
    drawbuff_pico.cpp
    file.cpp
    flash_cache.cpp
    font.cpp
    init.cpp
    input.cpp
    keyboard.cpp
    mem.cpp
    mouse.cpp
    net_select.cpp
    picosock.cpp
    timer.cpp
    wincomm.cpp
    window.cpp

    # a lot of SDLLIB isn't SDL specific (or is a stub)
    ../SDLLIB/buffer.cpp
    ../SDLLIB/dipthong.cpp
    ../SDLLIB/drawbuff.cpp
    ../SDLLIB/font.cpp
    ../SDLLIB/iconcach.cpp
    ../SDLLIB/iff.cpp
    ../SDLLIB/misc.cpp
    ../SDLLIB/palette.cpp
    ../SDLLIB/playcd.cpp
    ../SDLLIB/shape.cpp
    ../SDLLIB/wsa.cpp

    driver/audio.cpp
    driver/display.cpp
    driver/ppa_clut.c
    driver/usb.cpp
)

target_include_directories(esplib PUBLIC ../SDLLIB/include) # hax
target_include_directories(esplib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(esplib PRIVATE ${CMAKE_CURRENT_LIST_DIR}/driver)

target_compile_definitions(esplib PUBLIC TINY_BUILD ESP_BUILD)

target_link_libraries(esplib PUBLIC
    idf::newlib
    idf::fatfs
    idf::esp_timer
    idf::esp_lcd
    idf::esp_driver_ppa
    idf::usb
    idf::usb_host_hid
)
