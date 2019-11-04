#pragma once

#include <ek/config/detect_platform.hpp>

#if EK_MAC

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#elif EK_IOS

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#elif EK_ANDROID

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif EK_WEB

#define GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

enum : GLenum {
    GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL = 37441
};

inline void glBindVertexArray(GLuint array) {glBindVertexArrayOES(array);}
inline void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) {glDeleteVertexArraysOES(n, arrays);}
inline void glGenVertexArrays(GLsizei n, GLuint *arrays) {glGenVertexArraysOES(n, arrays);}

#elif EK_LINUX

#define GL_GLEXT_PROTOTYPES
//#include <GL/glcorearb.h>
//#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>

#endif

// Add some Desktop OpenGL features to OpenGL ES2

#if EK_ANDROID || EK_IOS

#define OPEN_GL_ES
#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef
#define glReadBuffer
#define glDrawBuffer
#define GL_FRAMEBUFFER_UNDEFINED 0x8219
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8

#endif

#ifndef GL_R8
#define GL_R8 (GL_R8_EXT)
#endif
#ifndef GL_RED
#define GL_RED (GL_RED_EXT)
#endif
#ifndef GL_RGBA8
#define GL_RGBA8 (GL_RGBA8_OES)
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F (GL_RGBA32F_EXT)
#endif