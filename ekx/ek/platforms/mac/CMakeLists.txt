message("Platform: macOS")

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_compile_definitions(${PROJECT_NAME} PUBLIC GL_SILENCE_DEPRECATION)

target_sources(${PROJECT_NAME} PRIVATE
        mac_app_delegate.h
        mac_app_delegate.mm
        mac_opengl_view.h
        mac_opengl_view.mm
        mac_sharing.cpp
        mac_utils.h
        mac_utils.mm

        # MOCK impl
        analytics.mm
        )

target_link_libraries(${PROJECT_NAME}
        PRIVATE "-framework Cocoa"
        PRIVATE "-framework OpenGL"
        PRIVATE "-framework CoreVideo"
        )

