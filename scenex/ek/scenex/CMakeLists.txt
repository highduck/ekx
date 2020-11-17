cmake_minimum_required(VERSION 3.13)

target_sources(${PROJECT_NAME} PRIVATE
        components/Node.cpp
        components/script.cpp
        components/script.hpp
        components/interactive.cpp
        components/layout.cpp
        components/event_handler.hpp

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
        systems/hitTest.cpp
        systems/hitTest.hpp

        AudioManager.cpp
        AudioManager.hpp
        InteractionSystem.cpp
        InteractionSystem.hpp

        particles/particle.cpp
        particles/particle.hpp
        particles/particle_decl.cpp
        particles/particle_decl.hpp
        particles/particle_system.cpp
        particles/particle_system.hpp

        utility/destroy_delay.hpp
        utility/destroy_delay.cpp
        utility/scene_management.hpp
        utility/basic_game_utility.hpp
        utility/basic_game_utility.cpp

        game_time.hpp
        game_time.cpp

        systems/main_flow.cpp
        systems/main_flow.hpp

        app/basic_application.cpp
        app/basic_application.hpp
        app/builtin_resources.cpp
        app/builtin_resources.hpp
        app/input_controller.hpp
        app/input_controller.cpp
        app/profiler.cpp

        3d/Transform3D.cpp
        3d/Transform3D.hpp
        3d/Camera3D.cpp
        3d/Camera3D.hpp
        3d/static_mesh.cpp
        3d/static_mesh.hpp
        3d/RenderSystem3D.cpp
        3d/RenderSystem3D.hpp
        3d/Light3D.cpp
        3d/Light3D.hpp
        3d/Material3D.cpp
        3d/Material3D.hpp

        local_storage_var_t.hpp

        2d/Atlas.cpp
        2d/Sprite.cpp
        2d/DynamicAtlas.cpp
        2d/Transform2D.cpp
        2d/Display2D.cpp
        2d/Camera2D.cpp
        2d/Camera2D_debug.cpp
        2d/UglyFilter2D.cpp
        2d/UglyFilter2D.hpp
        2d/RenderSystem2D.cpp
        2d/RenderSystem2D.hpp

        text/BitmapFont.cpp
        text/TrueTypeFont.cpp
        text/Font.cpp
        text/FontImplBase.cpp
        text/FontImplBase.hpp
        text/TextDrawer.cpp

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