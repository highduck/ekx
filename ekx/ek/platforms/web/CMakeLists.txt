message("Platform: Web")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE app_web.cpp)

target_compile_options(${PROJECT_NAME}
        PUBLIC
        -Wno-dollar-in-identifier-extension)

target_link_options(${PROJECT_NAME}
        PUBLIC
        --js-library ${CMAKE_CURRENT_SOURCE_DIR}/web.js
        )

target_link_libraries(${PROJECT_NAME} GLESv2)

add_custom_target(ek-web-js DEPENDS web.js)

add_dependencies(${PROJECT_NAME} ek-web-js)