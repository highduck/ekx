cmake_minimum_required(VERSION 3.13)

target_sources(${PROJECT_NAME} PRIVATE
        components/node_t.h
        components/transform_2d.cpp
        components/transform_2d.h
        config/ecs.h
        config.h
        scene_system.cpp
        scene_system.h
        components/script_t.cpp
        components/script_t.h
        components/display_2d.cpp
        components/display_2d.h
        components/name_t.h
        data/sg_factory.cpp
        data/sg_factory.h
        components/movie_t.h
        components/button_t.h
        systems/movie_clip_system.cpp
        systems/movie_clip_system.h
        components/interactive_t.h
        interactive_manager.cpp
        interactive_manager.h
        systems/button_system.cpp
        systems/button_system.h
        components/canvas.h
        systems/canvas_system.cpp
        systems/canvas_system.h

        ek/input_controller.h
        ek/input_controller.cpp
        simple_audio_manager.cpp
        simple_audio_manager.h
        components/event_handler.h
        components/layout.h
        systems/layout_system.cpp
        systems/layout_system.h
        particles/particle_layer.h
        particles/particle_emitter.h
        particles/particle_system.cpp
        particles/particle_system.h
        components/tween.h
        systems/tween_system.cpp
        systems/tween_system.h

        particles/particle.cpp
        particles/particle.h
        particles/particle_decl.cpp
        particles/particle_decl.h

        data/sg_data.cpp
        data/sg_data.h
        data/sg_loader.cpp

        components/node.cpp
        components/node_filters.cpp
        components/node_filters.h

        utility/destroy_delay.h
        utility/destroy_delay.cpp
        utility/scene_management.h

        goodies/shake_system.h
        goodies/shake_system.cpp

        systems/game_time.h
        systems/game_time.cpp
        systems/main_flow.cpp
        systems/main_flow.h

        ek/basic_application.cpp
        ek/basic_application.h
        ek/app_utils.cpp
        ek/app_utils.hpp

        goodies/helpers/mouse_follow_script.cpp
        goodies/helpers/mouse_follow_script.h
        goodies/helpers/target_follow_script.cpp
        goodies/helpers/target_follow_script.h
        goodies/helpers/trail_script.cpp
        goodies/helpers/trail_script.h

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
        )