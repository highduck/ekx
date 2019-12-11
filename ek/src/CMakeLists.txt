target_sources(${PROJECT_NAME} PUBLIC
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