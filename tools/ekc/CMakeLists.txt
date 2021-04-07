cmake_minimum_required(VERSION 3.13)
include(npm.cmake)
project(ekc)

add_executable(${PROJECT_NAME}
        src/cli_main.cpp

        src/ek/editor/marketing/export_marketing.hpp
        src/ek/editor/marketing/export_marketing.cpp

        src/ek/editor/obj/obj_loader.cpp
        src/ek/editor/obj/obj_loader.hpp

        src/ek/editor/assets/dynamic_atlas_editor_asset.cpp
        src/ek/editor/assets/MultiResAtlasEditorAsset.cpp
        src/ek/editor/assets/flash_asset.cpp
        src/ek/editor/assets/audio_asset.cpp
        src/ek/editor/assets/bitmap_font_editor_asset.cpp
        src/ek/editor/assets/texture_asset.cpp
        src/ek/editor/assets/model_asset.cpp
        src/ek/editor/assets/ttf_editor_asset.cpp
        src/ek/editor/assets/editor_project.cpp
        src/ek/editor/assets/editor_project.hpp
        src/ek/editor/assets/particles_asset.cpp
        src/ek/editor/assets/particles_asset.hpp
        src/ek/editor/assets/TranslationsAsset.hpp
        src/ek/editor/assets/TranslationsAsset.cpp
        src/ek/editor/assets/editor_asset.cpp
        src/ek/editor/assets/editor_asset.hpp

        src/ek/system/system.cpp
        src/ek/system/system.hpp
        src/ek/system/working_dir.cpp
        src/ek/system/working_dir.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g -Os)
endif ()

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::flash-export
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EKC_OS_PREFIX osx)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PUBLIC -Oz -flto)
        target_link_options(${PROJECT_NAME} PUBLIC -Wl,-dead_strip -Wl,-dead_strip_dylibs)
    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE shlwapi
            )
    set(EKC_OS_PREFIX win32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EKC_OS_PREFIX linux)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${EKC_OS_PREFIX}"
        RUNTIME_OUTPUT_NAME "ekc"
        RUNTIME_OUTPUT_NAME_DEBUG "ekc-debug"
        )