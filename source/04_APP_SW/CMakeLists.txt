##### APP_API/CMakeLists.txt #####

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
target_sources(${PROJECT_NAME} PRIVATE ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE SW_UTILS
    PRIVATE DEV_DRV
    PRIVATE PFM_SVC)