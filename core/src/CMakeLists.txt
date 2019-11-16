cmake_minimum_required(VERSION 3.12)
project(ek-core LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC

        ek/config/detect_platform.hpp

        ek/utility/common_macro.hpp
        ek/utility/strings.hpp
        ek/utility/strings.cpp
        ek/utility/static_ring_buffer.hpp
        ek/utility/static_history_buffer.hpp
        ek/utility/base64.cpp
        ek/utility/base64.hpp

        ek/binpack/max_rects.hpp
        ek/binpack/max_rects.cpp

        ek/math/internal/compiler_unsafe_begin.h
        ek/math/internal/compiler_unsafe_end.h
        ek/math/common.hpp
        ek/math/serialize_math.hpp

        ek/math/vec_fwd.hpp
        ek/math/matrix_fwd.hpp
        ek/math/vec.hpp
        ek/math/mat3x2.hpp
        ek/math/mat4x4.hpp
        ek/math/matrix_inverse.hpp
        ek/math/matrix_camera.hpp
        ek/math/matrix_transform.hpp
        ek/math/matrix_transpose.hpp
        ek/math/quaternion.hpp

        ek/math/box.hpp
        ek/math/rect.hpp
        ek/math/circle.hpp
        ek/math/bounds_builder.hpp

        ek/math/hvs_color.hpp
        ek/math/color_transform.hpp
        ek/math/packed_color.hpp

        ek/math/transform.hpp

        ek/math/easing.cpp
        ek/math/easing.hpp

        ek/math/collisions.hpp
        ek/math/collisions_sweep.cpp
        ek/math/collisions_test.cpp

        ek/math/rand.cpp
        ek/math/rand.hpp

        ek/math/internal/static_check.hpp
        ek/math/internal/static_check.cpp

        ek/logger.hpp
        ek/logger.cpp

        ek/array_buffer.hpp
        ek/assets.hpp
        ek/locator.hpp
        ek/signals.hpp
        ek/bit_vector.hpp

        ek/serialize/streams.hpp
        ek/serialize/core.hpp
        ek/serialize/types.hpp
        ek/serialize/serialize.hpp

        ek/timer.hpp
        ek/timer.cpp
        ek/fs/path.cpp
        ek/fs/path.hpp

        ek/imaging/image.cpp
        ek/imaging/image.hpp
        ek/imaging/drawing.cpp
        ek/imaging/drawing.hpp
        ek/imaging/filters.cpp
        ek/imaging/filters.hpp
        ek/imaging/decoder.cpp
        ek/imaging/decoder.hpp

        ek/utility/source_location.hpp

        # THIRD-PARTY
        stb/stb_image.h
        stb/stb_image.cpp
        )


target_include_directories(${PROJECT_NAME} PUBLIC .)

target_compile_options(${PROJECT_NAME} PUBLIC -std=c++17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
set_source_files_properties(stb/stb_image.cpp PROPERTIES COMPILE_FLAGS -Wno-cast-align)

set(EK_COMPILE_WARNINGS
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type
        -Woverloaded-virtual -Wcast-align -Wno-deprecated-declarations -Wstrict-aliasing)

set(EK_COMPILER_LTO "-flto")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(EK_COMPILE_WARNINGS ${EK_COMPILE_WARNINGS}
            -Wno-dollar-in-identifier-extension -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(EK_COMPILER_LTO "")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE
            -fno-exceptions -fstrict-aliasing -g ${EK_COMPILE_WARNINGS})
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Os -fno-exceptions -fstrict-aliasing ${EK_COMPILE_WARNINGS})
    if (${CMAKE_SYSTEM_NAME} MATCHES "Android" OR
            ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    else ()
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
        target_compile_options(${PROJECT_NAME} PRIVATE ${EK_COMPILER_LTO})
    endif ()
endif ()