##### SW_UTILS/CMakeLists.txt #####
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
)
get_filename_component(LAYER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

if(SRC)
    add_library(SW_UTILS STATIC)
    file(GLOB_RECURSE UTILS_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
    )

    target_sources(SW_UTILS PRIVATE ${UTILS_SOURCES})

    target_include_directories(SW_UTILS PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    add_library(SW_UTILS INTERFACE)
    target_include_directories(SW_UTILS INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
    message(STATUS "Skipping ${LAYER}: no sources found.")
endif()