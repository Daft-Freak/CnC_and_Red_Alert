cmake_minimum_required(VERSION 3.12)
project(CnCRedAlert)

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
add_subdirectory(SDLLIB)
add_subdirectory(WINVQ)

add_subdirectory(RA)
add_subdirectory(TD)
