message("Platform: Android")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE
        ek/android.hpp
        app_android.cpp
        )