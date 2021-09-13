message("Platform: LINUX")

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_sources(${PROJECT_NAME} PRIVATE app_linux.cpp)

if ($ENV{CLION_IDE})
    find_package(OpenGL)
else()
    find_package(OpenGL REQUIRED)
endif()

message(INFO "OPENGL_LIBRARIES => ${OPENGL_LIBRARIES}")
message(INFO "OPENGL_FOUND => ${OPENGL_FOUND}")
message(INFO "OPENGL_XMESA_FOUND => ${OPENGL_XMESA_FOUND}")
message(INFO "OPENGL_GLU_FOUND => ${OPENGL_GLU_FOUND}")
message(INFO "OpenGL_OpenGL_FOUND => ${OpenGL_OpenGL_FOUND}")
message(INFO "OpenGL_GLX_FOUND => ${OpenGL_GLX_FOUND}")
message(INFO "OpenGL_EGL_FOUND => ${OpenGL_EGL_FOUND}")
message(INFO "OPENGL_INCLUDE_DIR => ${OPENGL_INCLUDE_DIR}")
message(INFO "OPENGL_EGL_INCLUDE_DIRS => ${OPENGL_EGL_INCLUDE_DIRS}")

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${OPENGL_LIBRARIES}
        PRIVATE pthread
        PRIVATE ${CMAKE_DL_LIBS}
        PRIVATE X11
        PRIVATE Xi
        PRIVATE Xcursor
)
