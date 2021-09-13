target_sources(${PROJECT_NAME} PUBLIC
        ek/app/app.cpp
        ek/app/app.hpp
        ek/app/Platform.h
        ek/app/prefs.hpp
        ek/app/prefs.cpp
        ek/app/res.hpp

        ek/ext/analytics/analytics.hpp
        ek/ext/sharing/sharing.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)