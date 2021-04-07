target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        app_apple.h
        app_apple.mm
        )

target_compile_options(${PROJECT_NAME}
        PRIVATE -fobjc-arc
        PRIVATE -x objective-c++
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework QuartzCore"
            PRIVATE "-framework AudioToolbox")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE "-framework Foundation"
            PRIVATE "-framework UIKit"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework QuartzCore"
            PRIVATE "-framework AudioToolbox")
endif ()

