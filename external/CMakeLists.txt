cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_subdirectory(miniz)
add_subdirectory(pugixml)
add_subdirectory(imgui)
add_subdirectory(stb)
add_subdirectory(dr_libs)
add_subdirectory(miniaudio)
add_subdirectory(sokol)

include(FindFreetype)
message("FREETYPE INCLUDE: ${FREETYPE_INCLUDE_DIRS}")
message("FREETYPE LIBS: ${FREETYPE_LIBRARIES}")

add_library(freetype2 INTERFACE)
target_include_directories(freetype2 INTERFACE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(freetype2 INTERFACE ${FREETYPE_LIBRARIES})

find_package(Cairo)
message("CAIRO INCLUDE: ${CAIRO_INCLUDE_DIRS}")
message("CAIRO LIBS: ${CAIRO_LIBRARIES}")
if (CAIRO_FOUND)
    message("Cairo found")
else ()
    message("Cairo not found")
endif ()

add_library(cairo INTERFACE)
target_include_directories(cairo INTERFACE ${CAIRO_INCLUDE_DIRS})
target_link_libraries(cairo INTERFACE ${CAIRO_LIBRARIES})

if (EKX_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.8.0
    )
    FetchContent_GetProperties(googletest)
    if (NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    endif ()
endif ()