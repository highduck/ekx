cmake_minimum_required(VERSION 3.12)

project(native-lib LANGUAGES CXX)

add_library(${PROJECT_NAME} SHARED
        #-SOURCES-#
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        #-SEARCH_ROOTS-#
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
    target_compile_options(${PROJECT_NAME} PRIVATE -Os
            -fno-exceptions
            -ffunction-sections
            -fdata-sections
            -fvisibility=hidden)
    # https://github.com/llvm-mirror/llvm/blob/master/cmake/modules/AddLLVM.cmake
    target_link_options(${PROJECT_NAME} PUBLIC -Wl,--gc-sections)
endif ()

find_library(android-lib android)
find_library(gl-lib GLESv2)
find_library(log-lib log)

target_link_libraries(${PROJECT_NAME}
        ${log-lib}
        ${android-lib}
        ${gl-lib})