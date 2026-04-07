project(SANITY C CXX ASM)

pico_sdk_init()

add_executable(SANITY main.cpp)

pico_set_program_name(SANITY "SANITY")
pico_set_program_version(SANITY "0.1")

pico_enable_stdio_uart(SANITY 1)
pico_enable_stdio_usb(SANITY 1)

target_link_libraries(SANITY
    pico_stdlib
)

target_include_directories(SANITY PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(SANITY)