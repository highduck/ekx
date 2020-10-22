target_sources(${PROJECT_NAME} PUBLIC audiomini_web.cpp)

target_link_options(${PROJECT_NAME}
        PUBLIC
        --js-library ${CMAKE_CURRENT_SOURCE_DIR}/audiomini.js
        )