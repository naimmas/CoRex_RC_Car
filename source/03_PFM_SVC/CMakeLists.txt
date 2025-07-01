##### PFM_SVC/CMakeLists.txt #####
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
)
get_filename_component(LAYER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
if(SRC)
    add_library(PFM_SVC STATIC)

    file(GLOB_RECURSE SERVICE_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
    )

    target_sources(PFM_SVC PRIVATE ${SERVICE_SOURCES})

    target_include_directories(PFM_SVC PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Link to HAL library isolation from the rest of the project
    target_link_libraries(PFM_SVC     
        PRIVATE HW_API
        PUBLIC SW_UTILS)
else()
    add_library(PFM_SVC INTERFACE)
    message(STATUS "Skipping ${LAYER}: no sources found.")
endif()