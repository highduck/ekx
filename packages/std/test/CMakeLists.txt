cmake_minimum_required(VERSION 3.19)
project(std-tests)

add_executable(${PROJECT_NAME}
        ek_std_test.cpp
        math/math_easing_test.c
        math/math_mat_test.c
        math/math_rect_test.c
        math/math_test_common.h
        math/math_vec_test.c
        base64_test.c
        log_test.c
        path_test.c
        rnd_test.c
        string_test.c
        hash_test.c
        )

enable_testing()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
        #            LANGUAGE CXX
        C_STANDARD 11
        CXX_STANDARD 17
        )

file(GLOB_RECURSE all_c_test_files ${CMAKE_CURRENT_LIST_DIR}/*.c)

set_source_files_properties(${all_c_test_files} PROPERTIES LANGUAGE CXX)

target_compile_options(${PROJECT_NAME}
        #            PRIVATE -std=c++17
        #PRIVATE -xc++
        PUBLIC -g
        #PUBLIC -flto
        PUBLIC -O0
        PUBLIC -fno-strict-aliasing
#        PUBLIC -fno-exceptions
#        PUBLIC -fno-rtti
        PUBLIC -ffast-math
        PUBLIC -Wstrict-aliasing=2
        #            PUBLIC -fshort-enums
        )

#target_compile_definitions(${PROJECT_NAME}
#        PUBLIC DOCTEST_CONFIG_NO_EXCEPTIONS
#        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::std
        PUBLIC doctest
        )