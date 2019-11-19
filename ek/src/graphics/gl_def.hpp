#pragma once

#if defined(__EMSCRIPTEN__)
#define EK_WEBGL
#endif

#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if TARGET_OS_OSX

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#elif TARGET_OS_IOS

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#define EK_GLES2

#elif defined(__EMSCRIPTEN__) || defined(__ANDROID__)

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define EK_GLES2

#endif

//#if EK_LINUX
//
//#define GL_GLEXT_PROTOTYPES
////#include <GL/glcorearb.h>
////#include <GL/glx.h>
//#include <GL/gl.h>
//#include <GL/glext.h>
//
//#endif

#if defined(EK_GLES2)
// Add some Desktop OpenGL features to OpenGL ES2

#define glClearDepth glClearDepthf
#define glDepthRange glDepthRangef
#define glReadBuffer
#define glDrawBuffer

#endif

//#ifndef GL_R8
//#define GL_R8 (GL_R8_EXT)
//#endif
//#ifndef GL_RED
//#define GL_RED (GL_RED_EXT)
//#endif
#ifndef GL_RGBA8
#define GL_RGBA8 (GL_RGBA8_OES)
#endif
//#ifndef GL_RGBA32F
//#define GL_RGBA32F (GL_RGBA32F_EXT)
//#endif

#if GL_EXT_framebuffer_object
// GL_EXT_framebuffer_object

#undef glGenFramebuffers
#undef glGenRenderbuffers
#undef GL_FRAMEBUFFER
#undef GL_RENDERBUFFER
#undef glBindRenderbuffer
#undef glBindFramebuffer
#undef glRenderbufferStorage
#undef GL_DEPTH_ATTACHMENT
#undef GL_COLOR_ATTACHMENT0
#undef GL_FRAMEBUFFER_COMPLETE
#undef glFramebufferTexture2D
#undef glCheckFramebufferStatus
#undef glFramebufferRenderbuffer
#undef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#undef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#undef GL_FRAMEBUFFER_UNSUPPORTED
#undef glDeleteFramebuffers
#undef glDeleteRenderbuffers

#define glGenFramebuffers           glGenFramebuffersEXT
#define glGenRenderbuffers          glGenRenderbuffersEXT
#define GL_FRAMEBUFFER              GL_FRAMEBUFFER_EXT
#define GL_RENDERBUFFER             GL_RENDERBUFFER_EXT
#define glBindRenderbuffer          glBindRenderbufferEXT
#define glBindFramebuffer          glBindFramebufferEXT
#define glRenderbufferStorage       glRenderbufferStorageEXT
#define GL_DEPTH_ATTACHMENT       GL_DEPTH_ATTACHMENT_EXT
#define GL_COLOR_ATTACHMENT0       GL_COLOR_ATTACHMENT0_EXT
#define GL_FRAMEBUFFER_COMPLETE       GL_FRAMEBUFFER_COMPLETE_EXT
#define glFramebufferTexture2D       glFramebufferTexture2DEXT
#define glCheckFramebufferStatus       glCheckFramebufferStatusEXT
#define glFramebufferRenderbuffer       glFramebufferRenderbufferEXT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT       GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT       GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT
#define GL_FRAMEBUFFER_UNSUPPORTED       GL_FRAMEBUFFER_UNSUPPORTED_EXT
#define glDeleteFramebuffers       glDeleteFramebuffersEXT
#define glDeleteRenderbuffers       glDeleteRenderbuffersEXT

#endif
