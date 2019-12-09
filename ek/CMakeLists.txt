cmake_minimum_required(VERSION 3.12)
project(ek LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC

        src/platform/analytics.hpp
        src/platform/analytics.cpp

        src/platform/crash_reporter.hpp
        src/platform/game_center.hpp
        src/platform/sharing.hpp
        src/ek/app/app.cpp
        src/ek/app/app.hpp

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

        src/utils/image_loader.cpp
        src/utils/image_loader.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} ek-core)

target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES
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
    target_compile_options(${PROJECT_NAME}
            PRIVATE
            -fno-exceptions
            -fstrict-aliasing
            -g
            ${EK_COMPILE_WARNINGS}
            )
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Os -fno-exceptions
            -fstrict-aliasing
            ${EK_COMPILE_WARNINGS})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
            ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    else ()
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
        target_compile_options(${PROJECT_NAME} PRIVATE ${EK_COMPILER_LTO})
    endif ()
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    message("Platform: WASM")
    target_sources(${PROJECT_NAME} PRIVATE
            src/platform/static_resources.cpp

            platforms/web/web_app_impl.cpp
            platforms/web/audiomini_web.cpp

            platforms/null/GameServices.cpp
            )

    target_compile_options(${PROJECT_NAME}
            PUBLIC
            -Wno-dollar-in-identifier-extension)

    target_link_options(${PROJECT_NAME}
            PUBLIC
            --js-library ${PROJECT_SOURCE_DIR}/platforms/web/audiomini.js
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES iOS)

    message("Platform: IOS")

    target_include_directories(${PROJECT_NAME} PUBLIC platforms/apple)
    target_include_directories(${PROJECT_NAME} PUBLIC platforms/ios)

    target_sources(${PROJECT_NAME} PRIVATE
            src/platform/user_preferences.mm
            src/platform/static_resources.cpp
            src/platform/static_resources.mm
            src/platform/analytics.mm

            platforms/ios/ios_app_delegate.h
            platforms/ios/ios_app_delegate.mm
            platforms/ios/EAGLView.h
            platforms/ios/EAGLView.mm
            platforms/ios/ios_platform.mm
            platforms/ios/ios_sharing.mm
            platforms/ios/ios_game_center.mm

            platforms/apple/application.mm

            platforms/apple/audiomini_apple.mm
            platforms/ios/audiomini/CDAudioManager.h
            platforms/ios/audiomini/CDAudioManager.mm
            platforms/ios/audiomini/CDConfig.h
            platforms/ios/audiomini/CDOpenALSupport.h
            platforms/ios/audiomini/CDOpenALSupport.mm
            platforms/ios/audiomini/CocosDenshion.h
            platforms/ios/audiomini/CocosDenshion.mm
            platforms/ios/audiomini/SimpleAudioEngine_objc.h
            platforms/ios/audiomini/SimpleAudioEngine_objc.mm
            )

    target_link_libraries(${PROJECT_NAME}
            "-framework UIKit"
            "-framework OpenGLES"
            "-framework QuartzCore"
            "-framework AudioToolbox"
            "-framework Foundation"
            "-framework OpenAL")

    #target_compile_options(${PROJECT_NAME} PUBLIC -fno-aligned-allocation)
    target_compile_options(${PROJECT_NAME} PUBLIC -fobjc-arc)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLES_SILENCE_DEPRECATION)

    set_source_files_properties(
            platforms/ios/audiomini/CDAudioManager.mm
            platforms/ios/audiomini/CDOpenALSupport.mm
            platforms/ios/audiomini/CocosDenshion.mm
            platforms/ios/audiomini/SimpleAudioEngine_objc.mm
            PROPERTIES COMPILE_FLAGS -fno-objc-arc)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("Platform: MAC")
    target_include_directories(${PROJECT_NAME} PUBLIC platforms/apple)
    target_include_directories(${PROJECT_NAME} PUBLIC platforms/mac)
    target_compile_options(${PROJECT_NAME} PUBLIC -fobjc-arc)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GL_SILENCE_DEPRECATION)

    target_sources(${PROJECT_NAME} PRIVATE

            src/platform/user_preferences.mm
            src/platform/static_resources.cpp
            src/platform/static_resources.mm
            src/platform/analytics.mm

            platforms/null/GameServices.cpp

            platforms/mac/mac_opengl_view.mm
            platforms/mac/mac_opengl_view.h
            platforms/mac/mac_app_delegate.mm
            platforms/mac/mac_app_delegate.h
            platforms/mac/mac_window.mm
            platforms/mac/mac_sharing.cpp
            platforms/mac/mac_input.mm
            platforms/mac/mac_input.h

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

    target_link_libraries(${PROJECT_NAME}
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework CoreVideo"
            "-framework AudioToolbox"
            "-framework OpenAL"
            )

    set_source_files_properties(
            platforms/mac/audiomini/CDAudioManager.mm
            platforms/mac/audiomini/CDOpenALSupport.mm
            platforms/mac/audiomini/CDXMacOSXSupport.mm
            platforms/mac/audiomini/CocosDenshion.mm
            platforms/mac/audiomini/SimpleAudioEngine_objc.mm
            PROPERTIES COMPILE_FLAGS -fno-objc-arc)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    message("Platform: ANDROID")
    target_sources(${PROJECT_NAME} PRIVATE

            src/platform/static_resources.cpp
            platforms/android/ek/android.hpp

            platforms/android/android_main.cpp
            platforms/android/sharing.cpp
            platforms/android/android_game_center.cpp
            platforms/android/platform_jni.cpp
            platforms/android/audiomini_android.cpp
            )

    target_include_directories(${PROJECT_NAME} PUBLIC platforms/android)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message("Platform: WIN")
    target_sources(${PROJECT_NAME} PRIVATE
            platforms/null/audiomini_null.cpp
            platforms/null/sharing.cpp
            platforms/null/GameServices.cpp
            )

    target_compile_options(${PROJECT_NAME} PUBLIC
            -Wno-dollar-in-identifier-extension)

    find_package(OpenGL REQUIRED)
    if (NOT OPENGL_FOUND)
        message(ERROR "OPENGL not found!")
    endif ()

    target_include_directories(${PROJECT_NAME} PUBLIC ${OpenGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PUBLIC ${OpenGL_LIBRARY_DIRS})
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${OpenGL_DEFINITIONS})
    target_link_libraries(${PROJECT_NAME} -lwinmm -lglu32 ${OPENGL_LIBRARIES})

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message("Platform: LINUX")

    target_sources(ek PRIVATE
            src/platform/static_resources.cpp

            platforms/null/audiomini_null.cpp
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

    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} pthread)

else ()
    error("Platform: UNKNOWN")

endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()