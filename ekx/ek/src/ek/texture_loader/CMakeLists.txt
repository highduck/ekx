target_sources(${PROJECT_NAME} PRIVATE
        TextureLoader.h
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_options(${PROJECT_NAME}
            PUBLIC "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/ek_texture_loader.js"
            PUBLIC "SHELL:--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/../../../web/texture-loader.js"
            )
endif ()