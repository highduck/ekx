cmake_minimum_required(VERSION 3.13)
project(soloud)

add_library(${PROJECT_NAME} STATIC "")

add_subdirectory(include)
add_subdirectory(src)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

#set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/audiosource/wav/stb_vorbis.c PROPERTIES LANGUAGE CXX)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_compile_definitions(${PROJECT_NAME} PUBLIC
            WITH_SDL2_STATIC)
    target_link_libraries(${PROJECT_NAME}
            -lSDL
#            #"-framework OpenAL"
            )
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
#    add_subdirectory(apple)
#    add_subdirectory(ios)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_COREAUDIO)
    target_link_libraries(${PROJECT_NAME}
            "-framework AudioToolbox"
            #"-framework OpenAL"
            )
    #elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
#    add_subdirectory(platforms/android)
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#    add_subdirectory(null)
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#    add_subdirectory(null)
#else ()
#    error("Platform: UNKNOWN")
#
endif ()