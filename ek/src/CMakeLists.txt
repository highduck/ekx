target_sources(${PROJECT_NAME} PUBLIC
        draw2d/batcher.cpp
        draw2d/batcher.hpp
        draw2d/batch_state_manager.cpp
        draw2d/batch_state_manager.hpp
        draw2d/drawer.cpp
        draw2d/drawer.hpp

        graphics/gl_def.hpp
        graphics/gl_debug.cpp
        graphics/gl_debug.hpp
        graphics/blend_mode.hpp
        graphics/buffer_object.cpp
        graphics/buffer_object.hpp
        graphics/graphics.cpp
        graphics/graphics.hpp
        graphics/program.cpp
        graphics/program.hpp
        graphics/render_target.hpp
        graphics/render_target.cpp
        graphics/texture.cpp
        graphics/texture_load.cpp
        graphics/texture.hpp
        graphics/vertex_decl.cpp
        graphics/vertex_decl.hpp

        ek/app/app.cpp
        ek/app/app.hpp
        ek/app/prefs.hpp
        ek/app/res.hpp
        ek/app/internal/res_sys.hpp
        ek/app/internal/res_sys.cpp

        ek/audiomini.hpp

        ek/ext/analytics/analytics.hpp
        ek/ext/game_center/game_center.hpp
        ek/ext/sharing/sharing.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)