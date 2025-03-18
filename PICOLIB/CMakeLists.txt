add_library(picolib INTERFACE)

target_include_directories(picolib INTERFACE ../SDLLIB/include) # hax
target_include_directories(picolib INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# lwip usage is mostly so socket code compiles
target_link_libraries(picolib INTERFACE pico_stdlib pico_lwip pico_lwip_arch)
target_include_directories(picolib INTERFACE ${PICO_LWIP_PATH}/src/include/compat/posix)