cmake_minimum_required(VERSION 3.13)

target_sources(${PROJECT_NAME} PRIVATE
        popup_manager.cpp
        popup_manager.hpp
        shake_system.hpp
        shake_system.cpp

        simple_animator_script.h
        simple_animator_script.cpp
        fireworks_script.h
        fireworks_script.cpp

        helpers/mouse_follow_script.cpp
        helpers/mouse_follow_script.hpp
        helpers/target_follow_script.cpp
        helpers/target_follow_script.hpp
        helpers/trail_script.cpp
        helpers/trail_script.hpp
        )