add_library(picolib STATIC
    audio.cpp
    drawbuff_pico.cpp
    file.cpp
    flash_cache.cpp
    init.cpp
    keyboard.cpp
    mem.cpp
    mouse.cpp
    net_select.cpp
    picosock.cpp
    timer.cpp
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
    ../SDLLIB/wincomm.cpp
    ../SDLLIB/wsa.cpp

    # driver code borrows a lot from the 32blit-sdk
    driver/filesystem.cpp
    driver/psram.c
    driver/storage.cpp
    driver/usb_hid.cpp

    driver/fatfs/ff.c
    driver/fatfs/ffunicode.c
)

# "extra" board config (the thing the pico is plugged into)
if(NOT EXTRA_BOARD)
    message("No EXTRA_BOARD set, assuming Display Pack")
    set(EXTRA_BOARD display_pack)
endif()

string(TOUPPER ${EXTRA_BOARD} EXTRA_BOARD_UPPER)
target_compile_definitions(picolib PRIVATE EXTRA_BOARD_${EXTRA_BOARD_UPPER}=1)

if(EXTRA_BOARD STREQUAL "display_pack")
    set(DISPLAY_DRIVER "dbi")
endif()

target_sources(picolib PRIVATE
    driver/display_${DISPLAY_DRIVER}.cpp
)

# PIO programs
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/spi.pio)
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/dbi-spi.pio)
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/dbi-8bit.pio)

target_include_directories(picolib PUBLIC ../SDLLIB/include) # hax
target_include_directories(picolib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(picolib PRIVATE ${CMAKE_CURRENT_LIST_DIR}/driver)

target_link_libraries(picolib PUBLIC
    hardware_dma hardware_flash hardware_pio hardware_pwm
    pico_stdlib
    tinyusb_host
)

target_compile_definitions(picolib PUBLIC
    # Using a Qw/ST connector as a UART...
    PICO_DEFAULT_UART=1
    PICO_DEFAULT_UART_TX_PIN=4
    PICO_DEFAULT_UART_RX_PIN=5
    PICO_CORE1_STACK_SIZE=0x200
)

if(PICO_CYW43_SUPPORTED)
    target_link_libraries(picolib PUBLIC pico_cyw43_arch_lwip_threadsafe_background)
    target_compile_definitions(picolib PUBLIC WIFI_ENABLED)
endif()