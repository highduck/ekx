cmake_minimum_required(VERSION 3.13)
project(ek-core)

add_library(${PROJECT_NAME} STATIC
        ek/module.cpp

        ek/config.hpp

        ek/math/internal/compiler_unsafe_begin.h
        ek/math/internal/compiler_unsafe_end.h
        ek/math/common.hpp
        ek/math/serialize_math.hpp

        ek/math/max_rects.hpp
        ek/math/max_rects.cpp

        ek/math/vec.hpp
        ek/math/matrix_decl.hpp
        ek/math/mat.hpp
        ek/math/mat2x2.hpp
        ek/math/mat3x2.hpp
        ek/math/mat3x3.hpp
        ek/math/mat4x4.hpp
        ek/math/matrix_inverse.hpp
        ek/math/matrix_camera.hpp
        ek/math/matrix_transform.hpp
        ek/math/matrix_transpose.hpp
        ek/math/quaternion.hpp

        ek/math/box.hpp
        ek/math/circle.hpp
        ek/math/bounds_builder.hpp

        ek/math/hvs_color.hpp
        ek/math/color_transform.hpp
        ek/math/packed_color.hpp

        ek/math/easing.hpp

        ek/math/rand_impl.hpp
        ek/math/rand.hpp

        ek/math/internal/static_check.hpp

        ek/serialize/streams.hpp
        ek/serialize/core.hpp
        ek/serialize/types.hpp
        ek/serialize/serialize.hpp

        ek/imaging/image.cpp
        ek/imaging/image.hpp
        ek/imaging/drawing.cpp
        ek/imaging/drawing.hpp
        ek/imaging/decoder.cpp
        ek/imaging/decoder.hpp
        ek/imaging/fast_blur.cpp

        ek/util/Res.hpp
        ek/util/logger.hpp
        ek/util/logger.cpp
        ek/util/ServiceLocator.hpp
        ek/util/signals.hpp
        ek/util/base64.hpp
        ek/util/base64.cpp
        ek/util/detect_platform.hpp
        ek/util/common_macro.hpp
        ek/util/strings.hpp
        ek/util/strings.cpp
        ek/util/path.cpp
        ek/util/path.hpp
        ek/util/type_index.hpp

        ek/Localization.hpp
        ek/Localization.cpp

        ek/util/StaticStorage.hpp
        ek/Allocator.hpp
        ek/Allocator.cpp

        ek/ds/StaticRingBuffer.hpp
        ek/ds/StaticHistoryBuffer.hpp
        ek/ds/BitVector.hpp
        ek/ds/SmallArray.hpp
        ek/ds/Array.hpp
        ek/ds/SparseArray.hpp
        ek/ds/Hash.hpp

        ek/timers.hpp
        ek/timers_impl.hpp

        ecxx/impl/ecxx.cpp


        ek/imaging/ImageSubSystem.cpp
        ek/imaging/ImageSubSystem.hpp

        ek/impl/Arguments_impl.hpp

        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 17
        )

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    set_source_files_properties(stb/stb_image.cpp PROPERTIES COMPILE_FLAGS -Wno-cast-align) #

    set(EK_COMPILE_WARNINGS
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type
            -Woverloaded-virtual -Wcast-align -Wno-deprecated-declarations -Wstrict-aliasing)

    #set(EK_COMPILER_LTO "-flto")
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(EK_COMPILE_WARNINGS ${EK_COMPILE_WARNINGS}
            -Wno-dollar-in-identifier-extension -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    #set(EK_COMPILER_LTO "")
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fstrict-aliasing
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC stb
        PUBLIC sokol
        PUBLIC tracy
        )