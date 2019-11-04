cmake_minimum_required(VERSION 3.12)

add_library(${PROJECT_NAME} STATIC

        ek/editor/assets/flash_asset.cpp
        ek/editor/assets/flash_asset.hpp
        ek/editor/assets/audio_asset.cpp
        ek/editor/assets/audio_asset.hpp
        ek/editor/assets/freetype_asset.cpp
        ek/editor/assets/freetype_asset.hpp
        ek/editor/assets/editor_temp_atlas.cpp
        ek/editor/assets/editor_temp_atlas.hpp

        ek/editor/gui/gui_atlas.cpp
        ek/editor/gui/gui_sg_file.cpp
        ek/editor/gui/editor_widgets.hpp
        ek/editor/gui/gui_font.cpp
        ek/editor/gui/gui_asset_object_controls.cpp
        ek/editor/gui/gui_asset_manager.cpp
        ek/editor/gui/editor_assets.cpp
        ek/editor/gui/editor_assets.hpp
        ek/editor/gui/screen_recorder.cpp
        ek/editor/gui/screen_recorder.hpp
        ek/editor/gui/gui_window_stats.cpp
        ek/editor/gui/gui_window_stats.h
        ek/editor/gui/gui_window_tree.cpp
        ek/editor/gui/gui_window_tree.h
        ek/editor/gui/gui_dev_mode.cpp
        ek/editor/gui/gui_dev_mode.h

        ek/editor/imgui/imgui_module.hpp
        ek/editor/imgui/imgui_module.cpp
        ek/editor/imgui/imgui_stdlib.h
        ek/editor/imgui/imgui_stdlib.cpp
        ek/editor/imgui/imgui.cpp
        ek/editor/imgui/imgui.hpp

        ek/editor/editor.cpp
        ek/editor/editor.hpp
        ek/editor/assets/program_asset.cpp
        ek/editor/assets/program_asset.hpp

        plist/plist.hpp
        plist/plist.cpp
        plist/date.cpp
        plist/date.hpp
        plist/base64.hpp

        ekc/process_assets.cpp
        ekc/build_web.cpp
        ekc/project_config.cpp
        ekc/project_config.h
        ekc/project_config_detect.cpp
        ekc/project_config_templating.cpp
        ekc/template_vars.cpp
        ekc/template_vars.h
        ekc/process_asset_flash.cpp
        ekc/process_market_assets.hpp
        ekc/process_market_assets.cpp
        ekc/create_android.cpp
        ekc/create_xcode_ios.cpp
        ekc/generate_project_common.cpp
        ekc/generate_project_common.hpp
        ek/editor/gui/gui_ekc.cpp
        ekc/ekc.hpp
        ek/editor/assets/texture_asset.cpp
        ek/editor/assets/texture_asset.hpp
        ek/editor/assets/model_asset.cpp
        ek/editor/assets/model_asset.hpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_compile_definitions(${PROJECT_NAME} PUBLIC EK_EDITOR)
target_compile_options(${PROJECT_NAME} PUBLIC
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        #        -fno-exceptions
        )
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)

else ()
    #set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

    target_compile_options(${PROJECT_NAME} PRIVATE -flto -Os)

    # https://github.com/llvm-mirror/llvm/blob/master/cmake/modules/AddLLVM.cmake
    #    target_link_options(${PROJECT_NAME}
    #                        PRIVATE
    #                        -Wl,-dead_strip
    #                        -Wl,-dead_strip_dylibs
    #                        )
endif ()

target_link_libraries(${PROJECT_NAME}
        CONAN_PKG::kainjow-mustache
        CONAN_PKG::jsonformoderncpp
        CONAN_PKG::fmt
        CONAN_PKG::imgui
        ek-flash
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build")