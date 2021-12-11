cmake_minimum_required(VERSION 3.19)
project(pugixml)

add_library(${PROJECT_NAME} STATIC
        src/pugixml.cpp
        src/pugixml.hpp
        src/pugiconfig.hpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

target_compile_definitions(${PROJECT_NAME}
        PUBLIC PUGIXML_NO_XPATH
        PUBLIC PUGIXML_NO_STL
        PUBLIC PUGIXML_NO_EXCEPTIONS)
