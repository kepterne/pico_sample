cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_analog)

pico_sdk_init()

add_executable(pico_analog pico_analog.c common.c analog_reader.c lcd_display.c)

target_link_libraries(pico_analog hardware_adc)
target_link_libraries(pico_analog pico_stdlib)
target_link_libraries(pico_analog pico_multicore)
target_link_libraries(pico_analog hardware_dma)
target_link_libraries(pico_analog hardware_i2c)

pico_enable_stdio_usb(pico_analog 1)
pico_enable_stdio_uart(pico_analog 1)

pico_add_extra_outputs(pico_analog)

