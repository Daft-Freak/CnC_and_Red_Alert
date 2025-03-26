cmake_minimum_required(VERSION 3.12)

if(PICO_SDK_PATH)
    include(PICOLIB/pico_sdk_import.cmake)
endif()

project(CnCRedAlert)

if(PICO_SDK_PATH)
    pico_sdk_init()
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
    add_compile_definitions(-D__cdecl= -D__far= -D__stdcall= -Dcdecl= -Dfar=)
endif()

add_compile_definitions(-DWIN32) # hopefully easier than DOS


add_subdirectory(port)

if(PICO_SDK_PATH)
    add_subdirectory(PICOLIB)
else()
    add_subdirectory(SDLLIB)
endif()

add_subdirectory(WINVQ)

add_subdirectory(RA)

if(NOT PICO_SDK_PATH)
    add_subdirectory(TD)
    add_subdirectory(TOOLS/REMIX)
endif()
