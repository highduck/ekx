message("Platform: iOS")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        analytics.mm
        ios_app_delegate.h
        ios_app_delegate.mm
        ios_eagl_view.h
        ios_eagl_view.mm
        ios_game_center.mm
        ios_sharing.mm
        )

target_link_libraries(${PROJECT_NAME}
        "-framework UIKit"
        "-framework OpenGLES"
        "-framework QuartzCore"
        "-framework Foundation")

#target_compile_options(${PROJECT_NAME} PUBLIC -fno-aligned-allocation)
target_compile_definitions(${PROJECT_NAME} PUBLIC GLES_SILENCE_DEPRECATION)