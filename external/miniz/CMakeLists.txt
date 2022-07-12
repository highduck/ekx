cmake_minimum_required(VERSION 3.19)
project(miniz)

add_library(${PROJECT_NAME} STATIC
        src/miniz.c
        src/miniz.h
        src/miniz_export.h
        src/miniz_tdef.c
        src/miniz_tdef.h
        src/miniz_tinfl.c
        src/miniz_tinfl.h
        src/miniz_zip.c
        src/miniz_zip.h
        )

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (EKX_BUILD_EXTERNAL_TESTS)
    add_subdirectory(test)
endif ()
