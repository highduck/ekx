message("Platform: macOS")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        mac_app_delegate.h
        mac_app_delegate.mm
        mac_sharing.cpp
        mac_utils.h
        mac_utils.mm

        # MOCK impl
        analytics.mm
        )

target_compile_options(${PROJECT_NAME} PRIVATE -x objective-c++)

target_link_libraries(${PROJECT_NAME}
        PRIVATE "-framework QuartzCore"
        PRIVATE "-framework Cocoa"
        PRIVATE "-framework Metal"
        PRIVATE "-framework MetalKit"
        )

