cmake_minimum_required(VERSION 3.12)
project(ek LANGUAGES CXX)

add_library(ek STATIC

        src/platform/analytics.hpp
        src/platform/analytics.cpp

        src/platform/Ads.h
        src/platform/crash_reporter.hpp
        src/platform/GameServices.h
        src/platform/sharing.hpp
        src/platform/window.cpp
        src/platform/window.hpp
        src/platform/application.cpp
        src/platform/application.hpp

        src/draw2d/batcher.cpp
        src/draw2d/batcher.hpp
        src/draw2d/batch_state_manager.cpp
        src/draw2d/batch_state_manager.hpp
        src/draw2d/drawer.cpp
        src/draw2d/drawer.hpp

        src/graphics/gl_def.hpp
        src/graphics/gl_debug.cpp
        src/graphics/gl_debug.hpp
        src/graphics/blend_mode.hpp
        src/graphics/buffer_object.cpp
        src/graphics/buffer_object.hpp
        src/graphics/graphics.cpp
        src/graphics/graphics.hpp
        src/graphics/program.cpp
        src/graphics/program.hpp
        src/graphics/render_target.hpp
        src/graphics/render_target.cpp
        src/graphics/texture.cpp
        src/graphics/texture_load.cpp
        src/graphics/texture.hpp
        src/graphics/vertex_decl.cpp
        src/graphics/vertex_decl.hpp

        src/utils/basic_game_utility.cpp
        src/utils/basic_game_utility.hpp

        src/baseapp/base_app.cpp
        src/baseapp/base_app.hpp

        src/platform/user_preferences.hpp
        src/platform/user_preferences.cpp
        src/platform/static_resources.hpp

        src/ek/audiomini.hpp

        src/platform/boot.cpp
        src/platform/boot.h

        src/utils/image_loader.cpp
        src/utils/image_loader.hpp
        )

target_include_directories(ek PUBLIC src)
target_link_libraries(ek ek-core)

target_compile_options(ek PUBLIC -std=c++17)
set_target_properties(ek PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    set(EK_COMPILE_WARNINGS
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type -Woverloaded-virtual -Wcast-align
            -Wno-deprecated-declarations
            -DGL_SILENCE_DEPRECATION
            -Wstrict-aliasing)

    set(EK_COMPILER_LTO "-flto")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(EK_COMPILE_WARNINGS ${EK_COMPILE_WARNINGS}
            -Wno-dollar-in-identifier-extension
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(EK_COMPILER_LTO "")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(ek
            PRIVATE
            -fno-exceptions
            -fstrict-aliasing
            -g
            ${EK_COMPILE_WARNINGS}
            )
else ()
    target_compile_options(ek PRIVATE
            -Os -fno-exceptions
            -fstrict-aliasing
            ${EK_COMPILE_WARNINGS})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
            ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    else ()
        set_target_properties(ek PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
        target_compile_options(ek PRIVATE ${EK_COMPILER_LTO})
    endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    message("Platform: WASM")
    target_sources(ek PRIVATE
            src/platform/static_resources.cpp

            platforms/web/web_app_impl.cpp
            platforms/web/audiomini_web.cpp

            platforms/null/Ads.cpp
            platforms/null/GameServices.cpp
            )

    target_compile_options(ek
            PUBLIC
            -Wno-dollar-in-identifier-extension)

    target_link_options(ek
            PUBLIC
            --js-library ${PROJECT_SOURCE_DIR}/platforms/web/audiomini.js
            )
    #elseif ($ENV{CONAN_CMAKE_SYSTEM_NAME} MATCHES iOS)
elseif (${CONAN_SETTINGS_OS} MATCHES "iOS")

    message("Platform: IOS")

    target_include_directories(${PROJECT_NAME} PUBLIC platforms/apple)
    target_include_directories(${PROJECT_NAME} PUBLIC platforms/ios)
    #target_compile_options(${PROJECT_NAME} PUBLIC -fno-aligned-allocation)

    target_sources(${PROJECT_NAME} PRIVATE
            src/platform/user_preferences.mm
            src/platform/static_resources.cpp
            src/platform/static_resources.mm
            src/platform/analytics.mm

            platforms/ios/Ads.mm
            platforms/ios/main.mm
            platforms/ios/AppDelegate.h
            platforms/ios/AppDelegate.mm
            platforms/ios/EAGLView.h
            platforms/ios/EAGLView.mm
            platforms/ios/iosPlatform.mm
            platforms/ios/sharing.mm
            platforms/ios/GameServices.mm

            platforms/apple/application.mm

            platforms/apple/audiomini_apple.mm
            platforms/mac/audiomini/CDAudioManager.h
            platforms/mac/audiomini/CDAudioManager.mm
            platforms/mac/audiomini/CDConfig.h
            platforms/mac/audiomini/CDOpenALSupport.h
            platforms/mac/audiomini/CDOpenALSupport.mm
            platforms/mac/audiomini/CocosDenshion.h
            platforms/mac/audiomini/CocosDenshion.mm
            platforms/mac/audiomini/SimpleAudioEngine_objc.h
            platforms/mac/audiomini/SimpleAudioEngine_objc.mm
            )

    target_link_libraries(${PROJECT_NAME}
            "-framework UIKit"
            "-framework OpenGLES"
            "-framework QuartzCore"
            "-framework AudioToolbox"
            "-framework Foundation"
            "-framework OpenAL")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("Platform: MAC")
    target_include_directories(${PROJECT_NAME} PUBLIC platforms/apple)
    target_include_directories(${PROJECT_NAME} PUBLIC platforms/mac)
    target_sources(ek PRIVATE

            src/platform/user_preferences.mm
            src/platform/static_resources.cpp
            src/platform/static_resources.mm
            src/platform/analytics.mm

            platforms/null/Ads.cpp
            platforms/null/GameServices.cpp

            platforms/mac/main.mm
            platforms/mac/OpenGLView.mm
            platforms/mac/OpenGLView.h
            platforms/mac/AppDelegate.mm
            platforms/mac/AppDelegate.h
            platforms/mac/macPlatform.mm
            platforms/mac/sharing.cpp
            platforms/mac/osx_input.mm
            platforms/mac/osx_input.h

            platforms/apple/application.mm

            platforms/apple/audiomini_apple.mm
            platforms/mac/audiomini/CDAudioManager.h
            platforms/mac/audiomini/CDAudioManager.mm
            platforms/mac/audiomini/CDConfig.h
            platforms/mac/audiomini/CDOpenALSupport.h
            platforms/mac/audiomini/CDOpenALSupport.mm
            platforms/mac/audiomini/CDXMacOSXSupport.h
            platforms/mac/audiomini/CDXMacOSXSupport.mm
            platforms/mac/audiomini/CocosDenshion.h
            platforms/mac/audiomini/CocosDenshion.mm
            platforms/mac/audiomini/SimpleAudioEngine_objc.h
            platforms/mac/audiomini/SimpleAudioEngine_objc.mm
            )

    target_link_libraries(ek
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework CoreVideo"
            "-framework AudioToolbox"
            "-framework OpenAL"
            )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    message("Platform: ANDROID")
    target_sources(ek PRIVATE

            src/platform/static_resources.cpp
            platforms/android/ek/android.hpp

            platforms/android/android_main.cpp
            platforms/android/sharing.cpp
            platforms/android/GameServices.cpp
            platforms/android/Ads.cpp
            platforms/android/platform_jni.cpp
            platforms/android/audiomini_android.cpp
            )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message("Platform: WIN")
    #    target_sources(ek PRIVATE
    #                   platforms/null/ek/audiomini/AudioMini.cpp
    #                   platforms/null/Ads.cpp
    #                   platforms/null/GameServices.cpp
    #                   platforms/null/Sharing.cpp
    #                   )
    #
    #    target_compile_options(ek
    #                           PUBLIC
    #                           -Wno-dollar-in-identifier-extension)
    #
    #    find_package(OpenGL REQUIRED)
    #    target_link_libraries(ek GL)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("Platform: LINUX")

    target_sources(ek PRIVATE
            src/platform/static_resources.cpp

            platforms/null/audiomini_null.cpp
            platforms/null/Ads.cpp
            platforms/null/GameServices.cpp
            platforms/null/sharing.cpp
            )

    find_package(OpenGL)
    #    find_package(OpenGL REQUIRED)

    message(INFO "OPENGL_LIBRARIES => ${OPENGL_LIBRARIES}")
    message(INFO "OPENGL_FOUND => ${OPENGL_FOUND}")
    message(INFO "OPENGL_XMESA_FOUND => ${OPENGL_XMESA_FOUND}")
    message(INFO "OPENGL_GLU_FOUND => ${OPENGL_GLU_FOUND}")
    message(INFO "OpenGL_OpenGL_FOUND => ${OpenGL_OpenGL_FOUND}")
    message(INFO "OpenGL_GLX_FOUND => ${OpenGL_GLX_FOUND}")
    message(INFO "OpenGL_EGL_FOUND => ${OpenGL_EGL_FOUND}")
    message(INFO "OPENGL_INCLUDE_DIR => ${OPENGL_INCLUDE_DIR}")
    message(INFO "OPENGL_EGL_INCLUDE_DIRS => ${OPENGL_EGL_INCLUDE_DIRS}")

    target_link_libraries(ek ${OPENGL_LIBRARIES} pthread)

else ()
    error("Platform: UNKNOWN")

endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()