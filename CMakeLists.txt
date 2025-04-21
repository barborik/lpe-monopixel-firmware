cmake_minimum_required(VERSION 4.0)

set(PICO_BOARD pico_w)
set(PICO_CYW43_SUPPORTED 1)
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../FreeRTOS-Kernel)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(lpe-monopixel-firmware)

pico_sdk_init()

add_executable(lpe-monopixel-firmware
    src/main.c
    src/http.c
)

target_include_directories(lpe-monopixel-firmware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/conf/
)

target_link_libraries(lpe-monopixel-firmware
    pico_stdlib
    hardware_pwm
    hardware_adc
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(lpe-monopixel-firmware)
