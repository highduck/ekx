target_sources(${PROJECT_NAME} PRIVATE
        graphics.cpp
        graphics.hpp
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_options(${PROJECT_NAME}
            PUBLIC "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/ek_texture_loader.js"
            )
endif ()