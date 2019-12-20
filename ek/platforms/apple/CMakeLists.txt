target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        apple_common.h
        apple_common.mm
        static_resources.mm
        user_preferences.mm
        )

target_compile_options(${PROJECT_NAME} PRIVATE -fobjc-arc)
