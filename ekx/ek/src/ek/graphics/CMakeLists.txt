target_sources(${PROJECT_NAME} PRIVATE
        graphics.cpp
        graphics.hpp
        Helpers.hpp
        Helpers.cpp
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_property(SOURCE graphics.cpp PROPERTY COMPILE_FLAGS -x objective-c++)
endif ()