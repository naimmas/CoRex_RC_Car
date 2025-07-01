##### stm32/CMakeLists.txt #####

# Bring the auto-generated CubeMX interface
add_subdirectory(cmake/stm32cubemx)

# Wrap it in a static lib to force source compilation
add_library(MCU_HAL STATIC)
target_link_libraries(MCU_HAL PUBLIC stm32cubemx)

target_compile_definitions(MCU_HAL PUBLIC ${MCU_MODEL} USE_HAL_DRIVER)

target_compile_options(MCU_HAL PUBLIC
    $<$<CONFIG:Debug>: -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function>
    $<$<CONFIG:Release>: -Os -g0 -Wunused-variable -Wunused-parameter -Wunused-function>
)

target_link_libraries(MCU_HAL PRIVATE SW_UTILS)
# Link to final executable
target_link_libraries(${PROJECT_NAME} PUBLIC MCU_HAL)
