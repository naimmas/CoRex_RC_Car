##### HW_API/CMakeLists.txt #####

add_library(HW_API STATIC)

file(GLOB_RECURSE HW_API_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
)
target_sources(HW_API PRIVATE ${HW_API_SOURCES})

target_include_directories(HW_API PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to HAL library isolation from the rest of the project
target_link_libraries(HW_API 
    PRIVATE MCU_PORT
    PUBLIC SW_UTILS)