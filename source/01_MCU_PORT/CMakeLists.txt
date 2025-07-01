##### MCU_PORT/CMakeLists.txt #####

add_library(MCU_PORT STATIC)

file(GLOB_RECURSE PORT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/${MCU_TARGET}/*.c
)

target_sources(MCU_PORT PRIVATE ${PORT_SOURCES})

target_include_directories(MCU_PORT PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${MCU_TARGET}
    ${CMAKE_SOURCE_DIR}/02_HW_API
)

target_link_libraries(MCU_PORT PRIVATE
    MCU_HAL
    SW_UTILS
)