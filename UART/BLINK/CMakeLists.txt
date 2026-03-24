cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(BLINK C CXX ASM)

pico_sdk_init()

add_executable(BLINK
    BLINK.cpp
)

pico_set_program_name(BLINK "BLINK")
pico_set_program_version(BLINK "0.1")

target_link_libraries(BLINK
    pico_stdlib
)

target_include_directories(BLINK PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Serial over USB
pico_enable_stdio_usb(BLINK 1)
pico_enable_stdio_uart(BLINK 0)

pico_add_extra_outputs(BLINK)