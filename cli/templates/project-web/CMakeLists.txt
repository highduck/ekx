cmake_minimum_required(VERSION 3.19)

project(TEMPLATE_PROJECT_NAME)

add_executable(${PROJECT_NAME}
        #-SOURCES-#
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        #-SEARCH_ROOTS-#
        )

#{{{CMAKE_CODE}}}

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_link_options(${PROJECT_NAME} PUBLIC
        #-LINK_OPTIONS-#
        )

target_link_libraries(${PROJECT_NAME} GLESv2)

add_definitions("-ffast-math -fno-exceptions -fno-rtti -Wall -Wextra")

if (CMAKE_BUILD_TYPE MATCHES Debug)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -flto")
endif ()

#set_target_properties(${PROJECT_NAME}
#        PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

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

#-s FILESYSTEM=0
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
            "-s ENVIRONMENT=web -s FETCH=1 -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1"
            )
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
            "-s ENVIRONMENT=web -s FETCH=1 -s FILESYSTEM=0 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=0"
            )
    # --closure 1
endif ()
