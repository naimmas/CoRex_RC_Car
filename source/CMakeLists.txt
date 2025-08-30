cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0079 NEW)

# Select proper toolchain and MCU setup
set(MCU_LAYER_DIR ${CMAKE_SOURCE_DIR}/00_MCU_SDK)

set(CMAKE_TOOLCHAIN_FILE "${MCU_LAYER_DIR}/toolchain.cmake" CACHE FILEPATH "")

link_libraries(m) # Add math library globally

project(firmware C CXX ASM)

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define final executable
add_executable(${PROJECT_NAME})

# Subdirectories
add_subdirectory(SW_UTILS)
add_subdirectory(${MCU_LAYER_DIR})
add_subdirectory(01_MCU_PORT)
add_subdirectory(02_HW_API)
add_subdirectory(03_DEV_DRV)
add_subdirectory(03_PFM_SVC)
add_subdirectory(04_APP_SW)


if (NOT (CMAKE_SIZE AND CMAKE_OBJCOPY AND CMAKE_OBJDUMP))
    message(STATUS "CMAKE_SIZE: ${CMAKE_SIZE}")
    message(STATUS "CMAKE_OBJCOPY: ${CMAKE_OBJCOPY}")
    message(STATUS "CMAKE_OBJDUMP: ${CMAKE_OBJDUMP}")
    message(FATAL_ERROR "CMAKE_SIZE, CMAKE_OBJCOPY, and CMAKE_OBJDUMP must be set!")
endif()

# Post-build extras
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
COMMAND echo "Size of ${PROJECT_NAME}:"
COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}> > ${PROJECT_NAME}.size.txt
)

option(DUMP_ASM "Create full assembly of final executable" OFF)
if (${DUMP_ASM})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -D -C $<TARGET_FILE:${PROJECT_NAME}> > ${PROJECT_NAME}.s)
endif()