cmake_minimum_required(VERSION 3.19)
project(ekx)
#set(EKX_BUILD_DEV_TOOLS ON)
#set(EKX_BUILD_TESTS ON)

if ((NOT EMSCRIPTEN) AND (NOT (${CMAKE_SYSTEM_NAME} MATCHES "iOS")) AND (NOT (${CMAKE_SYSTEM_NAME} MATCHES "Android")))
    #)) AND () AND (${CMAKE_SYSTEM_NAME} NOT MATCHES "Android"))
    add_subdirectory(tools/bmfont-export)
    add_subdirectory(tools/flash-export)
    add_subdirectory(tools/obj-export)
    add_subdirectory(tools/sprite-packer)
endif ()

if (EKX_BUILD_DEV_TOOLS)
    add_subdirectory(packages/dev-tools)
endif ()

add_subdirectory(samples/rnd-test)
add_subdirectory(samples/sample-1-app)
add_subdirectory(samples/sample-2-drawing)

