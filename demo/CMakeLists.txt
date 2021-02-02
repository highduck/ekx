cmake_minimum_required(VERSION 3.13)
project(ek-demo)

add_executable(${PROJECT_NAME}
        src/demo_main.hpp
        src/demo_main.cpp
        src/3d/sample_3d.cpp
        src/3d/camera_arcball.cpp
        src/sample_text.cpp
        src/sample_base.cpp
        src/piko/piko.h
        src/piko/examples.h
        src/piko/examples.cpp
        src/piko/sample_piko.cpp
        src/sim/driver_node_script.cpp
        src/sim/driver_node_script.h
        src/sim/motion_system.cpp
        src/sim/motion_system.h
        src/sim/components/motion.h
        src/sim/components/attractor.h

        src/sample_flash.hpp
        src/sample_flash.cpp
        src/sample_integrations.cpp
        src/ui/minimal.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

#target_compile_definitions(${PROJECT_NAME} PUBLIC
#        )

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME}
        ekx
        )

add_definitions("-ffast-math -fno-exceptions -fno-rtti -Wall -Wextra")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#    add_definitions(-DNDEBUG)
else ()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Oz -ffast-math -fno-exceptions -fno-rtti -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -flto")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-dead_strip -Wl,-dead_strip_dylibs")
#    add_definitions(-DNDEBUG)
endif()

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

#if (CMAKE_BUILD_TYPE MATCHES Debug)
#    target_compile_options(${PROJECT_NAME} PRIVATE -g -fno-exceptions -fno-rtti)
#else ()
#    target_compile_options(${PROJECT_NAME} PRIVATE -g -Oz -fno-exceptions -fno-rtti) #-flto
#endif ()

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
            -s ALLOW_MEMORY_GROWTH=1
            -s FILESYSTEM=0
            )

    # defaults:
    # -s DISABLE_EXCEPTION_CATCHING=1
    # -s WASM_OBJECT_FILES=0
    # -s ASSERTIONS=1
    # TODO: strange runtime DOM exception error with Release, "-s STRICT=1"

    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
                "-s ENVIRONMENT=web -s FETCH=1 -s ALLOW_MEMORY_GROWTH=1 -s FILESYSTEM=0"
                )
    else ()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
                "-s ENVIRONMENT=web -s FETCH=1 -s ALLOW_MEMORY_GROWTH=1 -s FILESYSTEM=0 -s ASSERTIONS=0" # --closure 1
                )
        target_link_options(${PROJECT_NAME} PUBLIC
                --llvm-lto 1
                )
    endif ()

else ()
#    if (CMAKE_BUILD_TYPE MATCHES Debug)
#    else ()
#        # https://github.com/llvm-mirror/llvm/blob/master/cmake/modules/AddLLVM.cmake
#        target_link_options(${PROJECT_NAME}
#                PUBLIC
#                -Wl,-dead_strip
#                -Wl,-dead_strip_dylibs)
#    endif ()
endif ()
