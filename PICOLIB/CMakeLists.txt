add_library(picolib STATIC
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

    # driver code borrows a lot from the 32blit-sdk
    driver/dvi.c
    driver/filesystem.cpp
    driver/psram.c
    driver/storage.cpp
    driver/usb_hid.cpp

    driver/fatfs/ff.c
    driver/fatfs/ffunicode.c
)

# "extra" board config (the thing the pico is plugged into)
if(NOT EXTRA_BOARD)
    # no extra board needed
    if(PICO_BOARD STREQUAL adafruit_fruit_jam)
        set(EXTRA_BOARD "none")
    # assume stamp is in the carrier
    elseif(PICO_BOARD STREQUAL "solderparty_rp2350_stamp_xl")
        set(EXTRA_BOARD stamp_carrier)
    else()
        set(EXTRA_BOARD display_pack)
    endif()
    message("No EXTRA_BOARD set, assuming ${EXTRA_BOARD}")
endif()

string(TOUPPER ${EXTRA_BOARD} EXTRA_BOARD_UPPER)
target_compile_definitions(picolib PRIVATE EXTRA_BOARD_${EXTRA_BOARD_UPPER}=1)
set(AUDIO_DRIVER "none")

if(EXTRA_BOARD STREQUAL "display_pack")
    set(DISPLAY_DRIVER "dbi")
endif()

# this is slightly silly as it's a standalone thing
# but it doesn't have a config header in the SDK and there isn't a generic config to use for RP2350B either
if(EXTRA_BOARD STREQUAL "presto")
    set(DISPLAY_DRIVER "dpi")
endif()

if(EXTRA_BOARD STREQUAL "stamp_carrier")
    set(DISPLAY_DRIVER "hstx")
endif()

# "Stamp Display 2.8"
if(EXTRA_BOARD STREQUAL "sd28")
    set(DISPLAY_DRIVER "dbi")
    set(AUDIO_DRIVER "i2s")
endif()

if(EXTRA_BOARD STREQUAL "vgaboard")
    set(DISPLAY_DRIVER "dpi")
    set(AUDIO_DRIVER "i2s")
endif()

if(PICO_BOARD STREQUAL adafruit_fruit_jam)
    set(AUDIO_DRIVER "i2s")
    set(DISPLAY_DRIVER "hstx")
endif()

target_sources(picolib PRIVATE
    driver/display_${DISPLAY_DRIVER}.cpp
    driver/audio_${AUDIO_DRIVER}.cpp
)

# PIO programs
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/dbi-spi.pio)
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/dbi-8bit.pio)
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/dpi.pio)
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/i2s.pio)
pico_generate_pio_header(picolib ${CMAKE_CURRENT_LIST_DIR}/driver/spi.pio)

target_include_directories(picolib PUBLIC ../SDLLIB/include) # hax
target_include_directories(picolib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(picolib PRIVATE ${CMAKE_CURRENT_LIST_DIR}/driver)

target_link_libraries(picolib PUBLIC
    hardware_dma hardware_flash hardware_i2c hardware_pio hardware_pwm hardware_spi
    pico_multicore pico_stdlib
    tinyusb_host
)

# additionally link to pio usb if needed by board
if(PICO_BOARD STREQUAL "adafruit_fruit_jam")
    message("Using PIO USB host!")
    target_link_libraries(picolib PUBLIC tinyusb_pico_pio_usb)
    target_compile_definitions(picolib PUBLIC PIO_USB_HOST)
endif()

target_compile_definitions(picolib PUBLIC
    PICO_CORE1_STACK_SIZE=0x200
)

if(EXTRA_BOARD STREQUAL "display_pack")
    target_compile_definitions(picolib PUBLIC
        # Using a Qw/ST connector as a UART...
        PICO_DEFAULT_UART=1
        PICO_DEFAULT_UART_TX_PIN=4
        PICO_DEFAULT_UART_RX_PIN=5
    )
endif()

if(PICO_CYW43_SUPPORTED)
    target_link_libraries(picolib PUBLIC pico_cyw43_arch_lwip_threadsafe_background)
    target_compile_definitions(picolib PUBLIC WIFI_ENABLED)
endif()