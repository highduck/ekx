target_sources(${PROJECT_NAME} PUBLIC
        ek/audio/audio.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

add_subdirectory(ek/graphics)
add_subdirectory(ek/draw2d)