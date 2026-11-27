cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)

project(pihex-fw C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1)

pico_sdk_init()

add_executable(pihex-fw src/main.c)
target_sources(pihex-fw PRIVATE src/usb_descriptors.c)

target_include_directories(pihex-fw PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_definitions(pihex-fw PUBLIC CFG_TUSB_OS=OPT_OS_PICO)

target_link_directories(pihex-fw PRIVATE ${CMAKE_SOURCE_DIR}/stublibs)

target_link_libraries(pihex-fw
  pico_stdlib
  pico_unique_id
  tinyusb_device
  tinyusb_board
  hardware_spi
  hardware_dma
  hardware_pwm
)

pico_add_extra_outputs(pihex-fw)
