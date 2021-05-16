cmake_minimum_required(VERSION 3.13)

target_sources(${PROJECT_NAME} PRIVATE

        base/Node.hpp
        base/Node.cpp
        base/Script.hpp
        base/Script.cpp
        base/Interactive.hpp
        base/Interactive.cpp
        base/NodeEvents.hpp
        base/NodeEvents.cpp
        base/Tween.hpp
        base/Tween.cpp
        base/DestroyTimer.hpp
        base/DestroyTimer.cpp

        3d/Transform3D.cpp
        3d/Transform3D.hpp
        3d/StaticMesh.hpp
        3d/RenderSystem3D.cpp
        3d/RenderSystem3D.hpp
        3d/Camera3D.hpp
        3d/Light3D.hpp
        3d/Material3D.hpp

        2d/Atlas.cpp
        2d/Sprite.cpp
        2d/DynamicAtlas.cpp

        2d/Transform2D.cpp
        2d/Display2D.cpp
        2d/UglyFilter2D.cpp
        2d/UglyFilter2D.hpp
        2d/RenderSystem2D.cpp
        2d/RenderSystem2D.hpp
        2d/Camera2D.hpp
        2d/Camera2D.cpp
        2d/Camera2D_debug.cpp
        2d/LayoutRect.hpp
        2d/LayoutRect.cpp
        2d/Viewport.hpp
        2d/Viewport.cpp
        2d/Button.hpp
        2d/Button.cpp
        2d/MovieClip.hpp
        2d/MovieClip.cpp

        text/BitmapFont.cpp
        text/TrueTypeFont.cpp
        text/Font.cpp
        text/FontImplBase.cpp
        text/FontImplBase.hpp
        text/TextEngine.cpp

        AudioManager.cpp
        AudioManager.hpp
        InteractionSystem.cpp
        InteractionSystem.hpp

        particles/Particle.cpp
        particles/Particle.hpp
        particles/ParticleDecl.hpp
        particles/ParticleSystem.cpp
        particles/ParticleSystem.hpp

        app/basic_application.cpp
        app/basic_application.hpp
        app/input_controller.hpp
        app/input_controller.cpp
        app/profiler.cpp

        local_storage_var_t.hpp

        systems/hitTest.hpp
        systems/hitTest.cpp
        systems/main_flow.hpp
        systems/main_flow.cpp

        asset2/asset_object.hpp
        asset2/asset_manager.cpp
        asset2/asset_manager.hpp
        asset2/builtin_assets.cpp
        asset2/builtin_assets.hpp

        SceneFactory.hpp
        SceneFactory.cpp
        )