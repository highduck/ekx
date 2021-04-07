message("Platform: Windows")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE app_windows.cpp)

find_package(OpenGL REQUIRED)
if (NOT OPENGL_FOUND)
    message(ERROR "OPENGL not found!")
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC ${OpenGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${OpenGL_LIBRARY_DIRS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${OpenGL_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} -lwinmm -lglu32 ${OPENGL_LIBRARIES})