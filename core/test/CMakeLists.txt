cmake_minimum_required(VERSION 3.12)
project(ek-core-test)

add_executable(${PROJECT_NAME}
        serialize_test.cpp
        packed_color_test.cpp
        vec_test.cpp
        signal_test.cpp
        bit_vector_test.cpp
        string_utility_test.cpp
        base64_test.cpp
        image_test.cpp
        common_macro_test.cpp
        logger_test.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)

target_link_libraries(${PROJECT_NAME} CONAN_PKG::gtest ek-core)

add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})