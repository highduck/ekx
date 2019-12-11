cmake_minimum_required(VERSION 3.13)
project(ek-core-test)

add_executable(${PROJECT_NAME}
        ek/serialize/serialize.test.cpp
        ek/math/packed_color.test.cpp
        ek/math/vec.test.cpp
        ek/imaging/image.test.cpp
        ek/util/base64.test.cpp
        ek/util/bit_vector.test.cpp
        ek/util/strings.test.cpp
        ek/util/signals.test.cpp
        ek/util/logger.test.cpp
        ek/util/common_macro.test.cpp
        )

add_subdirectory(ecxx)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)

target_link_libraries(${PROJECT_NAME} gtest gtest_main ek-core)

add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})