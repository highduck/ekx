cmake_minimum_required(VERSION 3.13)
project(scenex-test)

add_executable(${PROJECT_NAME}
        asset_pack_utils_test.cpp
        dummy_text_format_test.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)

# TODO: `ek` dependency has precompiled MAIN entry-point
target_link_libraries(${PROJECT_NAME} gtest gtest_main scenex)
#target_include_directories(${PROJECT_NAME} PRIVATE ../src)

add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
        COMMAND ${PROJECT_NAME})