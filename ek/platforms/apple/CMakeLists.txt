target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        apple_common.h
        apple_common.mm
        audiomini_apple.mm
        static_resources.mm
        user_preferences.mm
        )

target_compile_options(${PROJECT_NAME} PUBLIC -fobjc-arc)
