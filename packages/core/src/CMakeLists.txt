cmake_minimum_required(VERSION 3.19)

project(core)

add_library(${PROJECT_NAME} STATIC
        ek/ek_core_module.cpp

        ek/math/Math.hpp
        ek/math/MathSerialize.hpp

        ek/math/HSVColor.hpp
        ek/math/Color32.hpp

        ek/math/Easings.hpp
        ek/math/Random.hpp

        ek/serialize/streams.hpp
        ek/serialize/core.hpp
        ek/serialize/types.hpp
        ek/serialize/serialize.hpp

        ek/util/Res.hpp
        ek/util/ServiceLocator.hpp
        ek/util/Signal.hpp
        ek/util/Platform.hpp
        ek/util/NoCopyAssign.hpp
        ek/util/StringUtil.hpp
        ek/util/Path.hpp

        ek/util/StaticStorage.hpp

        ek/ds/StaticRingBuffer.hpp
        ek/ds/StaticHistoryBuffer.hpp
        ek/ds/FixedArray.hpp
        ek/ds/Array.hpp
        ek/ds/Hash.hpp
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 11
        )

if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    set(EK_COMPILE_WARNINGS
            -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wnull-dereference -Wpedantic -Wreturn-type
            -Woverloaded-virtual -Wcast-align -Wno-deprecated-declarations -Wstrict-aliasing=2)

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
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::std
        PUBLIC stb
        PUBLIC sokol
        )