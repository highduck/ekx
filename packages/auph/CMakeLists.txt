cmake_minimum_required(VERSION 3.19)
include(npm.cmake)

project(auph)
add_library(${PROJECT_NAME} INTERFACE)
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE include)

target_link_libraries(${PROJECT_NAME}
        INTERFACE dr-libs
        INTERFACE stb
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${PROJECT_NAME}
            INTERFACE "-framework Foundation"
            INTERFACE "-framework CoreAudio"
            INTERFACE "-framework AudioToolbox"
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    target_link_libraries(${PROJECT_NAME}
            INTERFACE "-framework Foundation"
            INTERFACE "-framework AVFoundation"
            INTERFACE "-framework AudioToolbox"
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_include_directories(${PROJECT_NAME}
            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/android/oboe/include
            INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/android/oboe/src
            )
    target_link_libraries(${PROJECT_NAME}
            INTERFACE log
            INTERFACE android
            INTERFACE OpenSLES
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

endif ()

if (EMSCRIPTEN)
    target_link_options(${PROJECT_NAME}
            INTERFACE SHELL:--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/web/dist/emscripten/auph.js
            )
endif ()