cmake_minimum_required(VERSION 3.12)
project(ecxx-test LANGUAGES CXX)

add_executable(${PROJECT_NAME}

        common/components.hpp
        common/identity_generator_test_1.cpp
        common/identity_generator_test_2.cpp
        common/identity_generator_shared.hpp

        entity_pool_test.cpp
        entity_map_test.cpp
        bit_count_test.cpp
        sparse_vector_test.cpp
        entity_value_test.cpp
        world_test.cpp
        components_test.cpp
        view_test.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PUBLIC -fno-exceptions)
target_link_libraries(${PROJECT_NAME} ecxx CONAN_PKG::gtest)

enable_testing()
add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})