target_sources(${PROJECT_NAME} PUBLIC
        ek/app/app.cpp
        ek/app/app.hpp
        ek/app/device.hpp
        ek/app/prefs.hpp
        ek/app/prefs.cpp
        ek/app/res.hpp

        ek/ext/analytics/analytics.hpp
        ek/ext/sharing/sharing.hpp

        ek/audio/audio.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

add_subdirectory(ek/graphics)
add_subdirectory(ek/draw2d)