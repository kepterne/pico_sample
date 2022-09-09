cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(seatsensor)

pico_sdk_init()

add_executable(seatsensor seatsensor.c common.c)
target_link_libraries(seatsensor hardware_adc)
pico_enable_stdio_usb(seatsensor 1)
pico_enable_stdio_uart(seatsensor 1)

pico_add_extra_outputs(seatsensor)

target_link_libraries(seatsensor pico_stdlib)
