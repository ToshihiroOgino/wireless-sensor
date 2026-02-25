# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
set(PICO_BOARD pico_w CACHE STRING "Board type")

set(makefsdataTool ${USERHOME}/.pico-sdk/sdk/${sdkVersion}/src/rp2_common/pico_lwip/tools/makefsdata.py)
set(PICO_MAKEFSDATA_TOOL ${makefsdataTool} CACHE FILEPATH "Path to makefsdata.py")
set(FSDATA_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fsdata.c)


file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE HTTP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/html/*")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

set(PROJECT_NAME "wireless-sensor")

project(${PROJECT_NAME} C CXX ASM)

pico_sdk_init()


add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)
add_dependencies(${PROJECT_NAME} generate_fsdata)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_set_binary_type(${PROJECT_NAME} default)

find_package(Python3 REQUIRED COMPONENTS Interpreter)


add_custom_target(generate_fsdata ALL
    COMMAND ${Python3_EXECUTABLE} ${PICO_MAKEFSDATA_TOOL} -i ${HTTP_FILES} -o ${FSDATA_OUTPUT}
    DEPENDS ${HTTP_FILES} ${PICO_MAKEFSDATA_TOOL}
    BYPRODUCTS ${FSDATA_OUTPUT}
    COMMENT "Generating fsdata.c with makefsdata.py"
    VERBATIM
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(
    ${PROJECT_NAME}
    pico_stdlib
    hardware_i2c
    hardware_watchdog
    # pico_cyw43_arch_none
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_mdns
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

pico_add_extra_outputs(${PROJECT_NAME})
