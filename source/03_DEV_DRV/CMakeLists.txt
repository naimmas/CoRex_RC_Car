##### DEV_DRV/CMakeLists.txt #####

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
)
get_filename_component(LAYER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
if(SRC)
    add_library(DEV_DRV STATIC)

    file(GLOB_RECURSE DRIVER_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
    )
    target_sources(DEV_DRV PRIVATE ${DRIVER_SOURCES})

    target_include_directories(DEV_DRV PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(DEV_DRV
        PRIVATE HW_API
        PRIVATE PFM_SVC
        PUBLIC SW_UTILS)
else()
    add_library(DEV_DRV INTERFACE)
    message(STATUS "Skipping ${LAYER}: no sources found.")
endif()