message("Platform: Web")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE app_web.cpp)

target_compile_options(${PROJECT_NAME}
        PUBLIC -Wno-dollar-in-identifier-extension)

target_link_options(${PROJECT_NAME}
        PUBLIC SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/ekapp.js
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC GLESv2
        )

add_custom_target(ekapp-js DEPENDS ekapp.js)

add_dependencies(${PROJECT_NAME} ekapp-js)