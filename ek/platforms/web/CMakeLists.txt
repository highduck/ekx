message("Platform: Web")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        static_resources.cpp
        user_preferences.cpp
        web_app_impl.cpp

        # MOCK
        analytics.cpp
        game_services.cpp
        )

target_compile_options(${PROJECT_NAME}
        PUBLIC
        -Wno-dollar-in-identifier-extension)

target_link_options(${PROJECT_NAME}
        PUBLIC
        --js-library ${CMAKE_CURRENT_SOURCE_DIR}/web.js
        )

target_link_libraries(${PROJECT_NAME} GLESv2)