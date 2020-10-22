target_sources(${PROJECT_NAME} PUBLIC
        ek/app/app.cpp
        ek/app/app.hpp
        ek/app/device.hpp
        ek/app/prefs.hpp
        ek/app/res.hpp
        ek/app/internal/res_sys.hpp
        ek/app/internal/res_sys.cpp

        ek/ext/analytics/analytics.hpp
        ek/ext/game_center/game_center.hpp
        ek/ext/sharing/sharing.hpp

        raudio/raudio.h
        raudio/raudio.cpp
        raudio/dr_mp3.h
        raudio/miniaudio.h
        ek/audio/audio.hpp
        ek/audio/audio.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)