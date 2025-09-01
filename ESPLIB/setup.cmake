    include($ENV{IDF_PATH}/tools/cmake/idf.cmake)

    # since we're using "custom" cmake (or as I like to call it... cmake)
    # it doesn't seem like we can use the component manager
    # so fetch some components manually...
    include(FetchContent)

    FetchContent_Populate(esp_usb
        GIT_REPOSITORY https://github.com/espressif/esp-usb
        GIT_TAG f38a3c010f3d2897bf77c8c5a08240a2d5629593 # no tags so just pick a commit and hope it works
    )

    idf_build_component(${esp_usb_SOURCE_DIR}/host/class/hid/usb_host_hid/)

    # process build
    idf_build_process(${ESP_TARGET}
        SDKCONFIG ${CMAKE_CURRENT_LIST_DIR}/sdkconfig
        SDKCONFIG_DEFAULTS ${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults
    )

    # we need to build something here, and it needs to link to an idf:: library or we get an error of
    # "The SOURCES of "__idf_riscv" use a generator expression that depends on the SOURCES themselves."
    # for some reason (there aren't any generator expressions in SOURCES)
    add_executable(i-am-a-workaround ${CMAKE_CURRENT_LIST_DIR}/stub.c)
    target_link_libraries(i-am-a-workaround idf::newlib)
    idf_build_executable(i-am-a-workaround)