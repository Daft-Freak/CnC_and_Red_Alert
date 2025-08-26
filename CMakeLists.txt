cmake_minimum_required(VERSION 3.12)

if(PICO_SDK_PATH)
    include(PICOLIB/pico_sdk_import.cmake)
endif()

project(CnCRedAlert)

if(PICO_SDK_PATH)
    pico_sdk_init()
endif()

if(ESP_TARGET)
    include($ENV{IDF_PATH}/tools/cmake/idf.cmake)

    idf_build_process(${ESP_TARGET}
        SDKCONFIG ${CMAKE_CURRENT_LIST_DIR}/sdkconfig
        SDKCONFIG_DEFAULTS ${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults
    )

    # we need to build something here, and it needs to link to an idf:: library or we get an error of
    # "The SOURCES of "__idf_riscv" use a generator expression that depends on the SOURCES themselves."
    # for some reason (there aren't any generator expressions in SOURCES)
    add_executable(i-am-a-workaround stub.c)
    target_link_libraries(i-am-a-workaround idf::newlib)
    idf_build_executable(i-am-a-workaround)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(
        /wd4996 # yes, this code is old and uses "unsafe" functions
        /wd4068 /wd4081 # old #pragmas for another compiler
        /wd4661 # missing template definitions, 10000s of these
    )
else()
    # for noise
    add_compile_options(-Wno-write-strings)
    # workarounds
    add_compile_definitions(__cdecl= __far= __stdcall= cdecl= far=)
endif()

add_compile_definitions(WIN32) # hopefully easier than DOS


add_subdirectory(port)

if(PICO_SDK_PATH)
    add_subdirectory(PICOLIB)
elseif(ESP_TARGET)
    add_subdirectory(ESPLIB)
else()
    add_subdirectory(SDLLIB)
endif()

add_subdirectory(WINVQ)

add_subdirectory(RA)

add_subdirectory(TD)
