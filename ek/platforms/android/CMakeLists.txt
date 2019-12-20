message("Platform: Android")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        ek/android.hpp
        analytics.cpp
        android_game_center.cpp
        android_main.cpp
        sharing.cpp
        static_resources.cpp
        user_preferences.cpp
        )