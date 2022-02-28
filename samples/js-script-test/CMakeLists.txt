cmake_minimum_required(VERSION 3.19)
include(npm.cmake)
project(js-script-test)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/js_api.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

#target_compile_definitions(${PROJECT_NAME} PUBLIC
#        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::graphics
        PUBLIC ekx::audio
        PUBLIC ekx::std
        PUBLIC ekx::app
        PUBLIC ekx::quickjs
        PUBLIC ekx::res
        )

add_definitions("-ffast-math -fno-exceptions -fno-rtti -Wall -Wextra")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    #    add_definitions(-DNDEBUG)
else ()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Oz -ffast-math -fno-exceptions -fno-rtti -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -flto -g")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-dead_strip -Wl,-dead_strip_dylibs")
    #    add_definitions(-DNDEBUG)
endif ()

if(EK_UITEST)
    add_definitions(-DEK_UITEST)
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/export/uitest")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")
endif()