cmake_minimum_required(VERSION 3.13)

target_sources(${PROJECT_NAME} PRIVATE
        components/node.hpp
        components/node.cpp
        components/node_filters.cpp
        components/node_filters.hpp
        components/transform_2d.cpp
        components/transform_2d.hpp
        components/script.cpp
        components/script.hpp
        components/display_2d.cpp
        components/display_2d.hpp
        components/name.hpp
        components/movie.hpp
        components/button.hpp
        components/canvas.hpp
        components/interactive.hpp
        components/tween.hpp
        components/event_handler.hpp
        components/layout.hpp

        systems/movie_clip_system.cpp
        systems/movie_clip_system.hpp
        systems/button_system.cpp
        systems/button_system.hpp
        systems/canvas_system.cpp
        systems/canvas_system.hpp
        systems/layout_system.cpp
        systems/layout_system.hpp
        systems/tween_system.cpp
        systems/tween_system.hpp

        ek/input_controller.hpp
        ek/input_controller.cpp
        simple_audio_manager.cpp
        simple_audio_manager.hpp
        scene_system.cpp
        scene_system.hpp
        interactive_manager.cpp
        interactive_manager.hpp

        particles/particle.cpp
        particles/particle.hpp
        particles/particle_decl.cpp
        particles/particle_decl.hpp
        particles/particle_layer.hpp
        particles/particle_emitter.hpp
        particles/particle_system.cpp
        particles/particle_system.hpp

        utility/destroy_delay.hpp
        utility/destroy_delay.cpp
        utility/scene_management.hpp

        goodies/shake_system.hpp
        goodies/shake_system.cpp

        systems/game_time.hpp
        systems/game_time.cpp
        systems/main_flow.cpp
        systems/main_flow.hpp

        ek/basic_application.cpp
        ek/basic_application.hpp
        ek/app_utils.cpp
        ek/app_utils.hpp

        goodies/helpers/mouse_follow_script.cpp
        goodies/helpers/mouse_follow_script.hpp
        goodies/helpers/target_follow_script.cpp
        goodies/helpers/target_follow_script.hpp
        goodies/helpers/trail_script.cpp
        goodies/helpers/trail_script.hpp

        3d/transform_3d.cpp
        3d/transform_3d.hpp
        3d/camera_3d.cpp
        3d/camera_3d.hpp
        3d/static_mesh.cpp
        3d/static_mesh.hpp
        3d/render_system_3d.cpp
        3d/render_system_3d.hpp
        3d/light_3d.cpp
        3d/light_3d.hpp
        3d/material_3d.cpp
        3d/material_3d.hpp

        local_storage_var_t.hpp

        2d/atlas.cpp
        2d/atlas.hpp
        2d/sprite.cpp
        2d/sprite.hpp
        2d/font.cpp
        2d/font.hpp
        2d/text_format.hpp

        asset2/asset_object.hpp
        asset2/asset_manager.cpp
        asset2/asset_manager.hpp
        asset2/builtin_assets.cpp
        asset2/builtin_assets.hpp

        data/program_data.hpp
        data/texture_data.hpp
        data/model_data.hpp
        data/sg_data.cpp
        data/sg_data.hpp
        data/sg_loader.cpp
        data/sg_factory.cpp
        data/sg_factory.hpp
        )