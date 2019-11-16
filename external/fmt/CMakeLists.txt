cmake_minimum_required(VERSION 3.12)
project(fmt LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
        include/fmt/chrono.h
        include/fmt/color.h
        include/fmt/compile.h
        include/fmt/core.h
        include/fmt/format.h
        include/fmt/format-inl.h
        include/fmt/locale.h
        include/fmt/ostream.h
        include/fmt/posix.h
        include/fmt/printf.h
        src/format.cc
        src/posix.cc
        )

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_features(${PROJECT_NAME} INTERFACE
        cxx_auto_type cxx_variadic_templates)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        )
