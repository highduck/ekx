target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        apple_common.h
        apple_common.mm
        static_resources.mm
        user_preferences.mm
        device_apple.mm
        )

target_compile_options(${PROJECT_NAME} PRIVATE -fobjc-arc)
target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AudioToolbox")
