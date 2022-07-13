cmake_minimum_required(VERSION 3.19)
project(sample-2-drawing)

add_executable(${PROJECT_NAME} src/main.c)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::graphics
        )

add_definitions("-ffast-math -fno-exceptions -fno-rtti -Wall -Wextra")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -g")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -flto")
endif ()