cmake_minimum_required(VERSION 3.19)
project(obj-export)

add_executable(${PROJECT_NAME}
        src/obj_export.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g -Os)
endif ()

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::sg-file
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(OBJ_EXPORT_OS_PREFIX osx)

    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(EKC_COMPILE_FLAGS "-O2 -g -fno-exceptions -fno-rtti")
        set(EKC_LINK_FLAGS "-O2 -g")
    else ()
        set(EKC_COMPILE_FLAGS "-Oz -flto -DNDEBUG -fno-exceptions -fno-rtti")
        set(EKC_LINK_FLAGS "-Oz -flto -Wl,-dead_strip -Wl,-dead_strip_dylibs")
    endif ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EKC_COMPILE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EKC_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EKC_LINK_FLAGS}")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE shlwapi
            )
    set(OBJ_EXPORT_OS_PREFIX win32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(OBJ_EXPORT_OS_PREFIX linux)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
    endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/${OBJ_EXPORT_OS_PREFIX}"
        RUNTIME_OUTPUT_NAME "${PROJECT_NAME}"
        RUNTIME_OUTPUT_NAME_DEBUG "${PROJECT_NAME}-debug"
        )