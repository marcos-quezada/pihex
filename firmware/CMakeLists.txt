cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)

project(milestone0 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1)

pico_sdk_init()

add_executable(milestone0 src/main.c)

target_link_directories(milestone0 PRIVATE ${CMAKE_SOURCE_DIR}/stublibs)

target_link_libraries(milestone0
  pico_stdlib
  hardware_spi
  hardware_dma
  hardware_pwm
)

pico_add_extra_outputs(milestone0)
