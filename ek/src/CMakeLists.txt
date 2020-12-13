target_sources(${PROJECT_NAME} PUBLIC
        ek/app/app.cpp
        ek/app/app.hpp
        ek/app/device.hpp
        ek/app/prefs.hpp
        ek/app/res.hpp
        ek/app/internal/res_sys.hpp
        ek/app/internal/res_sys.cpp

        ek/ext/analytics/analytics.hpp
        ek/ext/sharing/sharing.hpp

        raudio/raudio.cpp

        ek/audio/audio.cpp

        ek/file_view.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)