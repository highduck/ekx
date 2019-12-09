cmake_minimum_required(VERSION 3.12)
project(picos LANGUAGES CXX)

add_executable(${PROJECT_NAME}
        src/picos_main.hpp
        src/picos_main.cpp
        src/piko/piko.h
        src/piko/examples.h
        src/piko/examples.cpp
        src/sim/driver_node_script.cpp
        src/sim/driver_node_script.h
        src/sim/motion_system.cpp
        src/sim/motion_system.h
        src/sim/components/position.h
        src/sim/components/motion.h
        src/sim/components/attractor.h
        src/camera_arcball.cpp
        src/camera_arcball.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        ekx
        )

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Oz -flto -fno-exceptions -fno-rtti)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    #-s SAFE_HEAP=1
    #-s ASSERTIONS=2
    #-s STACK_OVERFLOW_CHECK=2
    #-s ALIASING_FUNCTION_POINTERS=0
    #-s MODULARIZE=1
    set(EM_COMMON_FLAGS
            -s FETCH=1
            -s WASM=1
            -s WASM_ASYNC_COMPILATION=1
            -s DISABLE_EXCEPTION_CATCHING=1
            -s NO_EXIT_RUNTIME=1
            -s ALLOW_MEMORY_GROWTH=1
            -s FILESYSTEM=0
            )
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
                "-s FETCH=1 -s WASM=1 -s WASM_ASYNC_COMPILATION=1 -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s FILESYSTEM=0 -s ASSERTIONS=1"
                )
    else ()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
                "-s FETCH=1 -s WASM=1 -s WASM_ASYNC_COMPILATION=1 -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s FILESYSTEM=0 -s WASM_OBJECT_FILES=0 -s ASSERTIONS=0 --closure 1"
                )
        target_link_options(${PROJECT_NAME} PUBLIC
                --llvm-lto 1
                )
    endif ()

else ()
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        # https://github.com/llvm-mirror/llvm/blob/master/cmake/modules/AddLLVM.cmake
        target_link_options(${PROJECT_NAME}
                PUBLIC
                -Wl,-dead_strip
                -Wl,-dead_strip_dylibs)
    endif ()
endif ()
