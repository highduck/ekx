cmake_minimum_required(VERSION 3.19)

target_sources(${PROJECT_NAME} PRIVATE
        GameScreen.hpp
        GameScreen.cpp

        PopupManager.cpp
        PopupManager.hpp
        Shake.hpp
        Shake.cpp
        bubble_text.cpp
        bubble_text.hpp

        simple_animator_comp.h
        simple_animator_comp.cpp
        fireworks.h
        fireworks.cpp

        helpers/follow.cpp
        helpers/follow.h
        helpers/Trail2D.cpp
        helpers/Trail2D.hpp
        )