#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <ek/app/app.hpp>
#include <ek/assert.hpp>
#include <ek/util/logger.hpp>
#include "keymap_linux.h"

void ek_fail(const char* msg) {
    EK_ERROR << msg;
    abort();
}

#define X11_XDND_VERSION  5

#define GLX_VENDOR 1
#define GLX_RGBA_BIT 0x00000001
#define GLX_WINDOW_BIT 0x00000001
#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_RENDER_TYPE    0x8011
#define GLX_DOUBLEBUFFER 5
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_SAMPLES 0x186a1
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB 0x2094

typedef XID GLXWindow;
typedef XID GLXDrawable;
typedef struct __GLXFBConfig* GLXFBConfig;
typedef struct __GLXcontext* GLXContext;

typedef void (* __GLXextproc)(void);

typedef int (* PFNGLXGETFBCONFIGATTRIBPROC)(Display*, GLXFBConfig, int, int*);

typedef const char* (* PFNGLXGETCLIENTSTRINGPROC)(Display*, int);

typedef Bool (* PFNGLXQUERYEXTENSIONPROC)(Display*, int*, int*);

typedef Bool (* PFNGLXQUERYVERSIONPROC)(Display*, int*, int*);

typedef void (* PFNGLXDESTROYCONTEXTPROC)(Display*, GLXContext);

typedef Bool (* PFNGLXMAKECURRENTPROC)(Display*, GLXDrawable, GLXContext);

typedef void (* PFNGLXSWAPBUFFERSPROC)(Display*, GLXDrawable);

typedef const char* (* PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*, int);

typedef GLXFBConfig* (* PFNGLXGETFBCONFIGSPROC)(Display*, int, int*);

typedef __GLXextproc (* PFNGLXGETPROCADDRESSPROC)(const char* procName);

typedef void (* PFNGLXSWAPINTERVALEXTPROC)(Display*, GLXDrawable, int);

typedef XVisualInfo* (* PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*, GLXFBConfig);

typedef GLXWindow (* PFNGLXCREATEWINDOWPROC)(Display*, GLXFBConfig, Window, const int*);

typedef void (* PFNGLXDESTROYWINDOWPROC)(Display*, GLXWindow);

typedef int (* PFNGLXSWAPINTERVALMESAPROC)(int);

typedef GLXContext (* PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct AppXi {
    bool available;
    int major_opcode;
    int event_base;
    int error_base;
    int major;
    int minor;
};

struct AppXdnd {
    int version;
    Window source;
    Atom format;
    Atom XdndAware;
    Atom XdndEnter;
    Atom XdndPosition;
    Atom XdndStatus;
    Atom XdndActionCopy;
    Atom XdndDrop;
    Atom XdndFinished;
    Atom XdndSelection;
    Atom XdndTypeList;
    Atom text_uri_list;
};

struct AppX11 {
    uint8_t mouse_buttons;
    Display* display;
    int screen;
    Window root;
    Colormap colormap;
    Window window;
    Cursor hidden_cursor;
    int window_state;
    float dpi;
    unsigned char error_code;
    Atom UTF8_STRING;
    Atom WM_PROTOCOLS;
    Atom WM_DELETE_WINDOW;
    Atom WM_STATE;
    Atom NET_WM_NAME;
    Atom NET_WM_ICON_NAME;
    Atom NET_WM_STATE;
    Atom NET_WM_STATE_FULLSCREEN;
    AppXi xi;
    AppXdnd xdnd;
};

struct AppGLX {
    void* libgl;
    int major;
    int minor;
    int event_base;
    int error_base;
    GLXContext ctx;
    GLXWindow window;

    // GLX 1.3 functions
    PFNGLXGETFBCONFIGSPROC GetFBConfigs;
    PFNGLXGETFBCONFIGATTRIBPROC GetFBConfigAttrib;
    PFNGLXGETCLIENTSTRINGPROC GetClientString;
    PFNGLXQUERYEXTENSIONPROC QueryExtension;
    PFNGLXQUERYVERSIONPROC QueryVersion;
    PFNGLXDESTROYCONTEXTPROC DestroyContext;
    PFNGLXMAKECURRENTPROC MakeCurrent;
    PFNGLXSWAPBUFFERSPROC SwapBuffers;
    PFNGLXQUERYEXTENSIONSSTRINGPROC QueryExtensionsString;
    PFNGLXGETVISUALFROMFBCONFIGPROC GetVisualFromFBConfig;
    PFNGLXCREATEWINDOWPROC CreateWindow;
    PFNGLXDESTROYWINDOWPROC DestroyWindow;

    // GLX 1.4 and extension functions
    PFNGLXGETPROCADDRESSPROC GetProcAddress;
    PFNGLXGETPROCADDRESSPROC GetProcAddressARB;
    PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT;
    PFNGLXSWAPINTERVALMESAPROC SwapIntervalMESA;
    PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;

    // extension availability
    bool EXT_swap_control;
    bool MESA_swap_control;
    bool ARB_multisample;
    bool ARB_create_context;
    bool ARB_create_context_profile;
};

struct AppLinux {
    AppX11 x11;
    AppGLX glx;
};

static AppLinux gAppLinux;

struct gl_fbconfig {
    int red_bits;
    int green_bits;
    int blue_bits;
    int alpha_bits;
    int depth_bits;
    int stencil_bits;
    int samples;
    bool doublebuffer;
    uintptr_t handle;
};

void gl_init_fbconfig(gl_fbconfig* fbconfig) {
    memset(fbconfig, 0, sizeof(gl_fbconfig));
    /* -1 means "don't care" */
    fbconfig->red_bits = -1;
    fbconfig->green_bits = -1;
    fbconfig->blue_bits = -1;
    fbconfig->alpha_bits = -1;
    fbconfig->depth_bits = -1;
    fbconfig->stencil_bits = -1;
    fbconfig->samples = -1;
}

const gl_fbconfig* gl_choose_fbconfig(const gl_fbconfig* desired, const gl_fbconfig* alternatives, int count) {
    int missing, least_missing = 1000000;
    int color_diff, least_color_diff = 10000000;
    int extra_diff, least_extra_diff = 10000000;
    const gl_fbconfig* current;
    const gl_fbconfig* closest = 0;
    for (int i = 0; i < count; i++) {
        current = alternatives + i;
        if (desired->doublebuffer != current->doublebuffer) {
            continue;
        }
        missing = 0;
        if (desired->alpha_bits > 0 && current->alpha_bits == 0) {
            missing++;
        }
        if (desired->depth_bits > 0 && current->depth_bits == 0) {
            missing++;
        }
        if (desired->stencil_bits > 0 && current->stencil_bits == 0) {
            missing++;
        }
        if (desired->samples > 0 && current->samples == 0) {
            /* Technically, several multisampling buffers could be
                involved, but that's a lower level implementation detail and
                not important to us here, so we count them as one
            */
            missing++;
        }

        /* These polynomials make many small channel size differences matter
            less than one large channel size difference
            Calculate color channel size difference value
        */
        color_diff = 0;
        if (desired->red_bits != -1) {
            color_diff += (desired->red_bits - current->red_bits) * (desired->red_bits - current->red_bits);
        }
        if (desired->green_bits != -1) {
            color_diff += (desired->green_bits - current->green_bits) * (desired->green_bits - current->green_bits);
        }
        if (desired->blue_bits != -1) {
            color_diff += (desired->blue_bits - current->blue_bits) * (desired->blue_bits - current->blue_bits);
        }

        /* Calculate non-color channel size difference value */
        extra_diff = 0;
        if (desired->alpha_bits != -1) {
            extra_diff += (desired->alpha_bits - current->alpha_bits) * (desired->alpha_bits - current->alpha_bits);
        }
        if (desired->depth_bits != -1) {
            extra_diff += (desired->depth_bits - current->depth_bits) * (desired->depth_bits - current->depth_bits);
        }
        if (desired->stencil_bits != -1) {
            extra_diff +=
                    (desired->stencil_bits - current->stencil_bits) * (desired->stencil_bits - current->stencil_bits);
        }
        if (desired->samples != -1) {
            extra_diff += (desired->samples - current->samples) * (desired->samples - current->samples);
        }

        /* Figure out if the current one is better than the best one found so far
            Least number of missing buffers is the most important heuristic,
            then color buffer size match and lastly size match for other buffers
        */
        if (missing < least_missing) {
            closest = current;
        } else if (missing == least_missing) {
            if ((color_diff < least_color_diff) ||
                (color_diff == least_color_diff && extra_diff < least_extra_diff)) {
                closest = current;
            }
        }
        if (current == closest) {
            least_missing = missing;
            least_color_diff = color_diff;
            least_extra_diff = extra_diff;
        }
    }
    return closest;
}

using namespace ek;
using namespace ek::app;

int x11_error_handler(Display* display, XErrorEvent* event) {
    (void) (display);
    gAppLinux.x11.error_code = event->error_code;
    return 0;
}

void x11_grab_error_handler() {
    gAppLinux.x11.error_code = Success;
    XSetErrorHandler(x11_error_handler);
}

void x11_release_error_handler() {
    XSync(gAppLinux.x11.display, False);
    XSetErrorHandler(NULL);
}

void x11_init_extensions() {
    gAppLinux.x11.UTF8_STRING = XInternAtom(gAppLinux.x11.display, "UTF8_STRING", False);
    gAppLinux.x11.WM_PROTOCOLS = XInternAtom(gAppLinux.x11.display, "WM_PROTOCOLS", False);
    gAppLinux.x11.WM_DELETE_WINDOW = XInternAtom(gAppLinux.x11.display, "WM_DELETE_WINDOW", False);
    gAppLinux.x11.WM_STATE = XInternAtom(gAppLinux.x11.display, "WM_STATE", False);
    gAppLinux.x11.NET_WM_NAME = XInternAtom(gAppLinux.x11.display, "_NET_WM_NAME", False);
    gAppLinux.x11.NET_WM_ICON_NAME = XInternAtom(gAppLinux.x11.display, "_NET_WM_ICON_NAME", False);
    gAppLinux.x11.NET_WM_STATE = XInternAtom(gAppLinux.x11.display, "_NET_WM_STATE", False);
    gAppLinux.x11.NET_WM_STATE_FULLSCREEN = XInternAtom(gAppLinux.x11.display, "_NET_WM_STATE_FULLSCREEN", False);
    // TODO:
//    if (_sapp.drop.enabled) {
//        gAppLinux.x11.xdnd.XdndAware        = XInternAtom(gAppLinux.x11.display, "XdndAware", False);
//        gAppLinux.x11.xdnd.XdndEnter        = XInternAtom(gAppLinux.x11.display, "XdndEnter", False);
//        gAppLinux.x11.xdnd.XdndPosition     = XInternAtom(gAppLinux.x11.display, "XdndPosition", False);
//        gAppLinux.x11.xdnd.XdndStatus       = XInternAtom(gAppLinux.x11.display, "XdndStatus", False);
//        gAppLinux.x11.xdnd.XdndActionCopy   = XInternAtom(gAppLinux.x11.display, "XdndActionCopy", False);
//        gAppLinux.x11.xdnd.XdndDrop         = XInternAtom(gAppLinux.x11.display, "XdndDrop", False);
//        gAppLinux.x11.xdnd.XdndFinished     = XInternAtom(gAppLinux.x11.display, "XdndFinished", False);
//        gAppLinux.x11.xdnd.XdndSelection    = XInternAtom(gAppLinux.x11.display, "XdndSelection", False);
//        gAppLinux.x11.xdnd.XdndTypeList     = XInternAtom(gAppLinux.x11.display, "XdndTypeList", False);
//        gAppLinux.x11.xdnd.text_uri_list    = XInternAtom(gAppLinux.x11.display, "text/uri-list", False);
//    }

    /* check Xi extension for raw mouse input */
    if (XQueryExtension(gAppLinux.x11.display, "XInputExtension", &gAppLinux.x11.xi.major_opcode,
                        &gAppLinux.x11.xi.event_base, &gAppLinux.x11.xi.error_base)) {
        gAppLinux.x11.xi.major = 2;
        gAppLinux.x11.xi.minor = 0;
        if (XIQueryVersion(gAppLinux.x11.display, &gAppLinux.x11.xi.major, &gAppLinux.x11.xi.minor) == Success) {
            gAppLinux.x11.xi.available = true;
        }
    }
}

void x11_query_system_dpi() {
    /* from GLFW:
       NOTE: Default to the display-wide DPI as we don't currently have a policy
             for which monitor a window is considered to be on
        gAppLinux.x11.dpi = DisplayWidth(gAppLinux.x11.display, gAppLinux.x11.screen) *
                        25.4f / DisplayWidthMM(gAppLinux.x11.display, gAppLinux.x11.screen);
       NOTE: Basing the scale on Xft.dpi where available should provide the most
             consistent user experience (matches Qt, Gtk, etc), although not
             always the most accurate one
    */
    char* rms = XResourceManagerString(gAppLinux.x11.display);
    if (rms) {
        XrmDatabase db = XrmGetStringDatabase(rms);
        if (db) {
            XrmValue value;
            char* type = NULL;
            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
                if (type && strcmp(type, "String") == 0) {
                    gAppLinux.x11.dpi = atof(value.addr);
                }
            }
            XrmDestroyDatabase(db);
        }
    }
}

bool glx_has_ext(const char* ext, const char* extensions) {
    EK_ASSERT(ext);
    const char* start = extensions;
    while (true) {
        const char* where = strstr(start, ext);
        if (!where) {
            return false;
        }
        const char* terminator = where + strlen(ext);
        if ((where == start) || (*(where - 1) == ' ')) {
            if (*terminator == ' ' || *terminator == '\0') {
                break;
            }
        }
        start = terminator;
    }
    return true;
}

bool glx_extsupported(const char* ext, const char* extensions) {
    if (extensions) {
        return glx_has_ext(ext, extensions);
    } else {
        return false;
    }
}

void* glx_getprocaddr(const char* procname) {
    if (gAppLinux.glx.GetProcAddress) {
        return (void*) gAppLinux.glx.GetProcAddress(procname);
    } else if (gAppLinux.glx.GetProcAddressARB) {
        return (void*) gAppLinux.glx.GetProcAddressARB(procname);
    } else {
        return dlsym(gAppLinux.glx.libgl, procname);
    }
}

void glx_init() {
    const char* sonames[] = {"libGL.so.1", "libGL.so", 0};
    for (int i = 0; sonames[i]; i++) {
        gAppLinux.glx.libgl = dlopen(sonames[i], RTLD_LAZY | RTLD_GLOBAL);
        if (gAppLinux.glx.libgl) {
            break;
        }
    }
    if (!gAppLinux.glx.libgl) {
        ek_fail("GLX: failed to load libGL");
    }
    gAppLinux.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC) dlsym(gAppLinux.glx.libgl, "glXGetFBConfigs");
    gAppLinux.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC) dlsym(gAppLinux.glx.libgl, "glXGetFBConfigAttrib");
    gAppLinux.glx.GetClientString = (PFNGLXGETCLIENTSTRINGPROC) dlsym(gAppLinux.glx.libgl, "glXGetClientString");
    gAppLinux.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC) dlsym(gAppLinux.glx.libgl, "glXQueryExtension");
    gAppLinux.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC) dlsym(gAppLinux.glx.libgl, "glXQueryVersion");
    gAppLinux.glx.DestroyContext = (PFNGLXDESTROYCONTEXTPROC) dlsym(gAppLinux.glx.libgl, "glXDestroyContext");
    gAppLinux.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC) dlsym(gAppLinux.glx.libgl, "glXMakeCurrent");
    gAppLinux.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC) dlsym(gAppLinux.glx.libgl, "glXSwapBuffers");
    gAppLinux.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC) dlsym(gAppLinux.glx.libgl,
                                                                                  "glXQueryExtensionsString");
    gAppLinux.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC) dlsym(gAppLinux.glx.libgl, "glXCreateWindow");
    gAppLinux.glx.DestroyWindow = (PFNGLXDESTROYWINDOWPROC) dlsym(gAppLinux.glx.libgl, "glXDestroyWindow");
    gAppLinux.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC) dlsym(gAppLinux.glx.libgl, "glXGetProcAddress");
    gAppLinux.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC) dlsym(gAppLinux.glx.libgl, "glXGetProcAddressARB");
    gAppLinux.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC) dlsym(gAppLinux.glx.libgl,
                                                                                  "glXGetVisualFromFBConfig");
    if (!gAppLinux.glx.GetFBConfigs ||
        !gAppLinux.glx.GetFBConfigAttrib ||
        !gAppLinux.glx.GetClientString ||
        !gAppLinux.glx.QueryExtension ||
        !gAppLinux.glx.QueryVersion ||
        !gAppLinux.glx.DestroyContext ||
        !gAppLinux.glx.MakeCurrent ||
        !gAppLinux.glx.SwapBuffers ||
        !gAppLinux.glx.QueryExtensionsString ||
        !gAppLinux.glx.CreateWindow ||
        !gAppLinux.glx.DestroyWindow ||
        !gAppLinux.glx.GetProcAddress ||
        !gAppLinux.glx.GetProcAddressARB ||
        !gAppLinux.glx.GetVisualFromFBConfig) {
        ek_fail("GLX: failed to load required entry points");
    }

    if (!gAppLinux.glx.QueryExtension(gAppLinux.x11.display, &gAppLinux.glx.error_base, &gAppLinux.glx.event_base)) {
        ek_fail("GLX: GLX extension not found");
    }
    if (!gAppLinux.glx.QueryVersion(gAppLinux.x11.display, &gAppLinux.glx.major, &gAppLinux.glx.minor)) {
        ek_fail("GLX: Failed to query GLX version");
    }
    if (gAppLinux.glx.major == 1 && gAppLinux.glx.minor < 3) {
        ek_fail("GLX: GLX version 1.3 is required");
    }
    const char* exts = gAppLinux.glx.QueryExtensionsString(gAppLinux.x11.display, gAppLinux.x11.screen);
    if (glx_extsupported("GLX_EXT_swap_control", exts)) {
        gAppLinux.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC) glx_getprocaddr("glXSwapIntervalEXT");
        gAppLinux.glx.EXT_swap_control = 0 != gAppLinux.glx.SwapIntervalEXT;
    }
    if (glx_extsupported("GLX_MESA_swap_control", exts)) {
        gAppLinux.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC) glx_getprocaddr("glXSwapIntervalMESA");
        gAppLinux.glx.MESA_swap_control = 0 != gAppLinux.glx.SwapIntervalMESA;
    }
    gAppLinux.glx.ARB_multisample = glx_extsupported("GLX_ARB_multisample", exts);
    if (glx_extsupported("GLX_ARB_create_context", exts)) {
        gAppLinux.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC) glx_getprocaddr(
                "glXCreateContextAttribsARB");
        gAppLinux.glx.ARB_create_context = 0 != gAppLinux.glx.CreateContextAttribsARB;
    }
    gAppLinux.glx.ARB_create_context_profile = glx_extsupported("GLX_ARB_create_context_profile", exts);
}

int glx_attrib(GLXFBConfig fbconfig, int attrib) {
    int value;
    gAppLinux.glx.GetFBConfigAttrib(gAppLinux.x11.display, fbconfig, attrib, &value);
    return value;
}

GLXFBConfig glx_choosefbconfig() {
    GLXFBConfig* native_configs;
    gl_fbconfig* usable_configs;
    const gl_fbconfig* closest;
    int i, native_count, usable_count;
    const char* vendor;
    bool trust_window_bit = true;

    /* HACK: This is a (hopefully temporary) workaround for Chromium
           (VirtualBox GL) not setting the window bit on any GLXFBConfigs
    */
    vendor = gAppLinux.glx.GetClientString(gAppLinux.x11.display, GLX_VENDOR);
    if (vendor && strcmp(vendor, "Chromium") == 0) {
        trust_window_bit = false;
    }

    native_configs = gAppLinux.glx.GetFBConfigs(gAppLinux.x11.display, gAppLinux.x11.screen, &native_count);
    if (!native_configs || !native_count) {
        ek_fail("GLX: No GLXFBConfigs returned");
    }

    usable_configs = (gl_fbconfig*) calloc((size_t) native_count, sizeof(gl_fbconfig));
    usable_count = 0;
    for (i = 0; i < native_count; i++) {
        const GLXFBConfig n = native_configs[i];
        gl_fbconfig* u = usable_configs + usable_count;
        gl_init_fbconfig(u);

        /* Only consider RGBA GLXFBConfigs */
        if (0 == (glx_attrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT)) {
            continue;
        }
        /* Only consider window GLXFBConfigs */
        if (0 == (glx_attrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT)) {
            if (trust_window_bit) {
                continue;
            }
        }
        u->red_bits = glx_attrib(n, GLX_RED_SIZE);
        u->green_bits = glx_attrib(n, GLX_GREEN_SIZE);
        u->blue_bits = glx_attrib(n, GLX_BLUE_SIZE);
        u->alpha_bits = glx_attrib(n, GLX_ALPHA_SIZE);
        u->depth_bits = glx_attrib(n, GLX_DEPTH_SIZE);
        u->stencil_bits = glx_attrib(n, GLX_STENCIL_SIZE);
        if (glx_attrib(n, GLX_DOUBLEBUFFER)) {
            u->doublebuffer = true;
        }
        if (gAppLinux.glx.ARB_multisample) {
            u->samples = glx_attrib(n, GLX_SAMPLES);
        }
        u->handle = (uintptr_t) n;
        usable_count++;
    }
    gl_fbconfig desired;
    gl_init_fbconfig(&desired);
    desired.red_bits = 8;
    desired.green_bits = 8;
    desired.blue_bits = 8;
    desired.alpha_bits = 8;
    desired.depth_bits = 24;
    desired.stencil_bits = 8;
    desired.doublebuffer = true;
    desired.samples = 0;
    closest = gl_choose_fbconfig(&desired, usable_configs, usable_count);
    GLXFBConfig result = 0;
    if (closest) {
        result = (GLXFBConfig) closest->handle;
    }
    XFree(native_configs);
    free(usable_configs);
    return result;
}

void glx_choose_visual(Visual** visual, int* depth) {
    GLXFBConfig native = glx_choosefbconfig();
    if (0 == native) {
        ek_fail("GLX: Failed to find a suitable GLXFBConfig");
    }
    XVisualInfo* result = gAppLinux.glx.GetVisualFromFBConfig(gAppLinux.x11.display, native);
    if (!result) {
        ek_fail("GLX: Failed to retrieve Visual for GLXFBConfig");
    }
    *visual = result->visual;
    *depth = result->depth;
    XFree(result);
}

void glx_create_context() {
    GLXFBConfig native = glx_choosefbconfig();
    if (0 == native) {
        ek_fail("GLX: Failed to find a suitable GLXFBConfig (2)");
    }
    if (!(gAppLinux.glx.ARB_create_context && gAppLinux.glx.ARB_create_context_profile)) {
        ek_fail("GLX: ARB_create_context and ARB_create_context_profile required");
    }
    x11_grab_error_handler();
    const int attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            0, 0
    };
    gAppLinux.glx.ctx = gAppLinux.glx.CreateContextAttribsARB(gAppLinux.x11.display, native, NULL, True, attribs);
    if (!gAppLinux.glx.ctx) {
        ek_fail("GLX: failed to create GL context");
    }
    x11_release_error_handler();
    gAppLinux.glx.window = gAppLinux.glx.CreateWindow(gAppLinux.x11.display, native, gAppLinux.x11.window, NULL);
    if (!gAppLinux.glx.window) {
        ek_fail("GLX: failed to create window");
    }
}

void glx_destroy_context() {
    if (gAppLinux.glx.window) {
        gAppLinux.glx.DestroyWindow(gAppLinux.x11.display, gAppLinux.glx.window);
        gAppLinux.glx.window = 0;
    }
    if (gAppLinux.glx.ctx) {
        gAppLinux.glx.DestroyContext(gAppLinux.x11.display, gAppLinux.glx.ctx);
        gAppLinux.glx.ctx = 0;
    }
}

void glx_make_current() {
    gAppLinux.glx.MakeCurrent(gAppLinux.x11.display, gAppLinux.glx.window, gAppLinux.glx.ctx);
}

void glx_swap_buffers() {
    gAppLinux.glx.SwapBuffers(gAppLinux.x11.display, gAppLinux.glx.window);
}

void glx_swapinterval(int interval) {
    glx_make_current();
    if (gAppLinux.glx.EXT_swap_control) {
        gAppLinux.glx.SwapIntervalEXT(gAppLinux.x11.display, gAppLinux.glx.window, interval);
    } else if (gAppLinux.glx.MESA_swap_control) {
        gAppLinux.glx.SwapIntervalMESA(interval);
    }
}

void x11_send_event(Atom type, int a, int b, int c, int d, int e) {
    XEvent event;
    memset(&event, 0, sizeof(event));

    event.type = ClientMessage;
    event.xclient.window = gAppLinux.x11.window;
    event.xclient.format = 32;
    event.xclient.message_type = type;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(gAppLinux.x11.display, gAppLinux.x11.root,
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}

void x11_query_window_size() {
    XWindowAttributes attribs;
    XGetWindowAttributes(gAppLinux.x11.display, gAppLinux.x11.window, &attribs);

    using ::ek::app::g_app;
    g_app.window_size.x = attribs.width;
    g_app.window_size.y = attribs.height;
    g_app.drawable_size.x = g_app.window_size.x;
    g_app.drawable_size.y = g_app.window_size.y;
}

void x11_set_fullscreen(bool enable) {
    /* NOTE: this function must be called after XMapWindow (which happens in x11_show_window()) */
    if (gAppLinux.x11.NET_WM_STATE && gAppLinux.x11.NET_WM_STATE_FULLSCREEN) {
        if (enable) {
            const int _NET_WM_STATE_ADD = 1;
            x11_send_event(gAppLinux.x11.NET_WM_STATE,
                           _NET_WM_STATE_ADD,
                           gAppLinux.x11.NET_WM_STATE_FULLSCREEN,
                           0, 1, 0);
        } else {
            const int _NET_WM_STATE_REMOVE = 0;
            x11_send_event(gAppLinux.x11.NET_WM_STATE,
                           _NET_WM_STATE_REMOVE,
                           gAppLinux.x11.NET_WM_STATE_FULLSCREEN,
                           0, 1, 0);
        }
    }
    XFlush(gAppLinux.x11.display);
}

void x11_create_hidden_cursor() {
    EK_ASSERT(0 == gAppLinux.x11.hidden_cursor);
    const int w = 16;
    const int h = 16;
    XcursorImage* img = XcursorImageCreate(w, h);
    EK_ASSERT(img && (img->width == 16) && (img->height == 16) && img->pixels);
    img->xhot = 0;
    img->yhot = 0;
    const size_t num_bytes = (size_t) (w * h) * sizeof(XcursorPixel);
    memset(img->pixels, 0, num_bytes);
    gAppLinux.x11.hidden_cursor = XcursorImageLoadCursor(gAppLinux.x11.display, img);
    XcursorImageDestroy(img);
}

void x11_toggle_fullscreen() {
//    g_app.fullscreen = !g_app.fullscreen;
//    x11_set_fullscreen(g_app.fullscreen);
    x11_query_window_size();
}

void x11_show_mouse(bool show) {
    if (show) {
        XUndefineCursor(gAppLinux.x11.display, gAppLinux.x11.window);
    } else {
        XDefineCursor(gAppLinux.x11.display, gAppLinux.x11.window, gAppLinux.x11.hidden_cursor);
    }
}

void x11_lock_mouse(bool lock) {
//    if (lock == g_app.mouse.locked) {
//        return;
//    }
//    g_app.mouse.dx = 0.0f;
//    g_app.mouse.dy = 0.0f;
//    g_app.mouse.locked = lock;
//    if (g_app.mouse.locked) {
//        if (gAppLinux.x11.xi.available) {
//            XIEventMask em;
//            unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 }; // XIMaskLen is a macro
//            em.deviceid = XIAllMasterDevices;
//            em.mask_len = sizeof(mask);
//            em.mask = mask;
//            XISetMask(mask, XI_RawMotion);
//            XISelectEvents(gAppLinux.x11.display, gAppLinux.x11.root, &em, 1);
//        }
//        XGrabPointer(gAppLinux.x11.display, // display
//                     gAppLinux.x11.window,           // grab_window
//                     True,                       // owner_events
//                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,    // event_mask
//                     GrabModeAsync,              // pointer_mode
//                     GrabModeAsync,              // keyboard_mode
//                     gAppLinux.x11.window,           // confine_to
//                     gAppLinux.x11.hidden_cursor,    // cursor
//                     CurrentTime);               // time
//    }
//    else {
//        if (gAppLinux.x11.xi.available) {
//            XIEventMask em;
//            unsigned char mask[] = { 0 };
//            em.deviceid = XIAllMasterDevices;
//            em.mask_len = sizeof(mask);
//            em.mask = mask;
//            XISelectEvents(gAppLinux.x11.display, gAppLinux.x11.root, &em, 1);
//        }
//        XWarpPointer(gAppLinux.x11.display, None, gAppLinux.x11.window, 0, 0, 0, 0, (int) g_app.mouse.x, g_app.mouse.y);
//        XUngrabPointer(gAppLinux.x11.display, CurrentTime);
//    }
//    XFlush(gAppLinux.x11.display);
}

void x11_update_window_title() {
    const char* title = g_app.window_cfg.title.c_str();
    size_t titleLen = g_app.window_cfg.title.size();
    Xutf8SetWMProperties(gAppLinux.x11.display,
                         gAppLinux.x11.window,
                         title,
                         title,
                         NULL, 0, NULL, NULL, NULL);
    XChangeProperty(gAppLinux.x11.display, gAppLinux.x11.window,
                    gAppLinux.x11.NET_WM_NAME, gAppLinux.x11.UTF8_STRING, 8,
                    PropModeReplace,
                    (unsigned char*) title,
                    titleLen);
    XChangeProperty(gAppLinux.x11.display, gAppLinux.x11.window,
                    gAppLinux.x11.NET_WM_ICON_NAME, gAppLinux.x11.UTF8_STRING, 8,
                    PropModeReplace,
                    (unsigned char*) title,
                    titleLen);
    XFlush(gAppLinux.x11.display);
}

void x11_create_window(Visual* visual, int depth) {
    gAppLinux.x11.colormap = XCreateColormap(gAppLinux.x11.display, gAppLinux.x11.root, visual, AllocNone);
    XSetWindowAttributes wa;
    memset(&wa, 0, sizeof(wa));
    const uint32_t wamask = CWBorderPixel | CWColormap | CWEventMask;
    wa.colormap = gAppLinux.x11.colormap;
    wa.border_pixel = 0;
    wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                    PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                    ExposureMask | FocusChangeMask | VisibilityChangeMask |
                    EnterWindowMask | LeaveWindowMask | PropertyChangeMask;
    x11_grab_error_handler();
    gAppLinux.x11.window = XCreateWindow(gAppLinux.x11.display,
                                         gAppLinux.x11.root,
                                         0, 0,
                                         (uint32_t) g_app.window_size.x,
                                         (uint32_t) g_app.window_size.y,
                                         0,     /* border width */
                                         depth, /* color depth */
                                         InputOutput,
                                         visual,
                                         wamask,
                                         &wa);
    x11_release_error_handler();
    if (!gAppLinux.x11.window) {
        ek_fail("X11: Failed to create window");
    }
    Atom protocols[] = {
            gAppLinux.x11.WM_DELETE_WINDOW
    };
    XSetWMProtocols(gAppLinux.x11.display, gAppLinux.x11.window, protocols, 1);

    XSizeHints* hints = XAllocSizeHints();
    hints->flags |= PWinGravity;
    hints->win_gravity = StaticGravity;
    XSetWMNormalHints(gAppLinux.x11.display, gAppLinux.x11.window, hints);
    XFree(hints);

    /* announce support for drag'n'drop */
    // TODO:
//    if (g_app.drop.enabled) {
//        const Atom version = X11_XDND_VERSION;
//        XChangeProperty(gAppLinux.x11.display, gAppLinux.x11.window, gAppLinux.x11.xdnd.XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*) &version, 1);
//    }

    x11_update_window_title();
}

void x11_destroy_window() {
    if (gAppLinux.x11.window) {
        XUnmapWindow(gAppLinux.x11.display, gAppLinux.x11.window);
        XDestroyWindow(gAppLinux.x11.display, gAppLinux.x11.window);
        gAppLinux.x11.window = 0;
    }
    if (gAppLinux.x11.colormap) {
        XFreeColormap(gAppLinux.x11.display, gAppLinux.x11.colormap);
        gAppLinux.x11.colormap = 0;
    }
    XFlush(gAppLinux.x11.display);
}

bool x11_window_visible() {
    XWindowAttributes wa;
    XGetWindowAttributes(gAppLinux.x11.display, gAppLinux.x11.window, &wa);
    return wa.map_state == IsViewable;
}

void x11_show_window() {
    if (!x11_window_visible()) {
        XMapWindow(gAppLinux.x11.display, gAppLinux.x11.window);
        XRaiseWindow(gAppLinux.x11.display, gAppLinux.x11.window);
        XFlush(gAppLinux.x11.display);
    }
}

void x11_hide_window() {
    XUnmapWindow(gAppLinux.x11.display, gAppLinux.x11.window);
    XFlush(gAppLinux.x11.display);
}

unsigned long x11_get_window_property(Window window, Atom property, Atom type, unsigned char** value) {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;
    XGetWindowProperty(gAppLinux.x11.display,
                       window,
                       property,
                       0,
                       LONG_MAX,
                       False,
                       type,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       value);
    return itemCount;
}

int x11_get_window_state() {
    int result = WithdrawnState;
    struct {
        CARD32 state;
        Window icon;
    } * state = NULL;

    if (x11_get_window_property(gAppLinux.x11.window, gAppLinux.x11.WM_STATE, gAppLinux.x11.WM_STATE,
                                (unsigned char**) &state) >= 2) {
        result = (int) state->state;
    }
    if (state) {
        XFree(state);
    }
    return result;
}

// TODO:
uint32_t x11_mod(uint32_t x11_mods) {
    uint32_t mods = 0;
    if (x11_mods & ShiftMask) {
        //mods |= SAPP_MODIFIER_SHIFT;
    }
    if (x11_mods & ControlMask) {
        //mods |= SAPP_MODIFIER_CTRL;
    }
    if (x11_mods & Mod1Mask) {
        //mods |= SAPP_MODIFIER_ALT;
    }
    if (x11_mods & Mod4Mask) {
        //mods |= SAPP_MODIFIER_SUPER;
    }
    return mods;
}

void x11_app_event(event_type type) {
    event_t ev{type};
    dispatch_event(ev);
}

mouse_button x11_translate_button(const XEvent* event) {
    switch (event->xbutton.button) {
        case Button1:
            return mouse_button::left;
            //case Button2: return mouse_button::other;
        case Button3:
            return mouse_button::right;
        default:
            return mouse_button::other;
    }
}

void x11_mouse_event(event_type type, mouse_button btn, uint32_t mods) {
//    if (_sapp_events_enabled()) {
//        init_event(type);
//        _sapp.event.mouse_button = btn;
//        _sapp.event.modifiers = mods;
//        call_event(&_sapp.event);
//    }
}

void x11_scroll_event(float x, float y, uint32_t mods) {
//    if (_sapp_events_enabled()) {
//        init_event(SAPP_EVENTTYPE_MOUSE_SCROLL);
//        _sapp.event.modifiers = mods;
//        _sapp.event.scroll_x = x;
//        _sapp.event.scroll_y = y;
//        call_event(&_sapp.event);
//    }
}

void x11_key_event(event_t type, key_code key, bool repeat, uint32_t mods) {
//    if (_sapp_events_enabled()) {
//        init_event(type);
//        _sapp.event.key_code = key;
//        _sapp.event.key_repeat = repeat;
//        _sapp.event.modifiers = mods;
//        call_event(&_sapp.event);
//        /* check if a CLIPBOARD_PASTED event must be sent too */
//        if (_sapp.clipboard.enabled &&
//            (type == SAPP_EVENTTYPE_KEY_DOWN) &&
//            (_sapp.event.modifiers == SAPP_MODIFIER_CTRL) &&
//            (_sapp.event.key_code == SAPP_KEYCODE_V))
//        {
//            init_event(SAPP_EVENTTYPE_CLIPBOARD_PASTED);
//            call_event(&_sapp.event);
//        }
//    }
}

void x11_char_event(uint32_t chr, bool repeat, uint32_t mods) {
//if (_sapp_events_enabled()) {
//_sapp_init_event(SAPP_EVENTTYPE_CHAR);
//_sapp.event.char_code = chr;
//_sapp.event.key_repeat = repeat;
//_sapp.event.modifiers = mods;
//_sapp_call_event(&_sapp.event);
//}
}

key_code x11_translate_key(int scancode) {
    int dummy;
    KeySym* keysyms = XGetKeyboardMapping(gAppLinux.x11.display, scancode, 1, &dummy);
    EK_ASSERT(keysyms);
    KeySym keysym = keysyms[0];
    XFree(keysyms);
    switch (keysym) {
        case XK_Escape:
            return key_code::Escape;
        case XK_Tab:
            return key_code::Tab;
//        case XK_Shift_L:        return SAPP_KEYCODE_LEFT_SHIFT;
//        case XK_Shift_R:        return SAPP_KEYCODE_RIGHT_SHIFT;
//        case XK_Control_L:      return SAPP_KEYCODE_LEFT_CONTROL;
//        case XK_Control_R:      return SAPP_KEYCODE_RIGHT_CONTROL;
//        case XK_Meta_L:
//        case XK_Alt_L:          return SAPP_KEYCODE_LEFT_ALT;
//        case XK_Mode_switch:    /* Mapped to Alt_R on many keyboards */
//        case XK_ISO_Level3_Shift: /* AltGr on at least some machines */
//        case XK_Meta_R:
//        case XK_Alt_R:          return SAPP_KEYCODE_RIGHT_ALT;
//        case XK_Super_L:        return SAPP_KEYCODE_LEFT_SUPER;
//        case XK_Super_R:        return SAPP_KEYCODE_RIGHT_SUPER;
//        case XK_Menu:           return SAPP_KEYCODE_MENU;
//        case XK_Num_Lock:       return SAPP_KEYCODE_NUM_LOCK;
//        case XK_Caps_Lock:      return SAPP_KEYCODE_CAPS_LOCK;
//        case XK_Print:          return SAPP_KEYCODE_PRINT_SCREEN;
//        case XK_Scroll_Lock:    return SAPP_KEYCODE_SCROLL_LOCK;
//        case XK_Pause:          return SAPP_KEYCODE_PAUSE;
//        case XK_Delete:         return SAPP_KEYCODE_DELETE;
//        case XK_BackSpace:      return SAPP_KEYCODE_BACKSPACE;

        case XK_Return:
            return key_code::Enter;
        case XK_Home:
            return key_code::Home;
        case XK_End:
            return key_code::End;
        case XK_Page_Up:
            return key_code::PageUp;
        case XK_Page_Down:
            return key_code::PageDown;
        case XK_Insert:
            return key_code::Insert;
        case XK_Left:
            return key_code::ArrowLeft;
        case XK_Right:
            return key_code::ArrowRight;
        case XK_Down:
            return key_code::ArrowDown;
        case XK_Up:
            return key_code::ArrowUp;
//        case XK_F1:             return key_code::F1;
//        case XK_F2:             return SAPP_KEYCODE_F2;
//        case XK_F3:             return SAPP_KEYCODE_F3;
//        case XK_F4:             return SAPP_KEYCODE_F4;
//        case XK_F5:             return SAPP_KEYCODE_F5;
//        case XK_F6:             return SAPP_KEYCODE_F6;
//        case XK_F7:             return SAPP_KEYCODE_F7;
//        case XK_F8:             return SAPP_KEYCODE_F8;
//        case XK_F9:             return SAPP_KEYCODE_F9;
//        case XK_F10:            return SAPP_KEYCODE_F10;
//        case XK_F11:            return SAPP_KEYCODE_F11;
//        case XK_F12:            return SAPP_KEYCODE_F12;
//        case XK_F13:            return SAPP_KEYCODE_F13;
//        case XK_F14:            return SAPP_KEYCODE_F14;
//        case XK_F15:            return SAPP_KEYCODE_F15;
//        case XK_F16:            return SAPP_KEYCODE_F16;
//        case XK_F17:            return SAPP_KEYCODE_F17;
//        case XK_F18:            return SAPP_KEYCODE_F18;
//        case XK_F19:            return SAPP_KEYCODE_F19;
//        case XK_F20:            return SAPP_KEYCODE_F20;
//        case XK_F21:            return SAPP_KEYCODE_F21;
//        case XK_F22:            return SAPP_KEYCODE_F22;
//        case XK_F23:            return SAPP_KEYCODE_F23;
//        case XK_F24:            return SAPP_KEYCODE_F24;
//        case XK_F25:            return SAPP_KEYCODE_F25;

//        case XK_KP_Divide:      return SAPP_KEYCODE_KP_DIVIDE;
//        case XK_KP_Multiply:    return SAPP_KEYCODE_KP_MULTIPLY;
//        case XK_KP_Subtract:    return SAPP_KEYCODE_KP_SUBTRACT;
//        case XK_KP_Add:         return SAPP_KEYCODE_KP_ADD;

//        case XK_KP_Insert:      return SAPP_KEYCODE_KP_0;
//        case XK_KP_End:         return SAPP_KEYCODE_KP_1;
//        case XK_KP_Down:        return SAPP_KEYCODE_KP_2;
//        case XK_KP_Page_Down:   return SAPP_KEYCODE_KP_3;
//        case XK_KP_Left:        return SAPP_KEYCODE_KP_4;
//        case XK_KP_Begin:       return SAPP_KEYCODE_KP_5;
//        case XK_KP_Right:       return SAPP_KEYCODE_KP_6;
//        case XK_KP_Home:        return SAPP_KEYCODE_KP_7;
//        case XK_KP_Up:          return SAPP_KEYCODE_KP_8;
//        case XK_KP_Page_Up:     return SAPP_KEYCODE_KP_9;
//        case XK_KP_Delete:      return SAPP_KEYCODE_KP_DECIMAL;
//        case XK_KP_Equal:       return SAPP_KEYCODE_KP_EQUAL;
//        case XK_KP_Enter:       return SAPP_KEYCODE_KP_ENTER;

        case XK_a:
            return key_code::A;
            //case XK_b:              return key_code::B;
        case XK_c:
            return key_code::C;
        case XK_d:
            return key_code::D;
//        case XK_e:              return key_code::E;
//        case XK_f:              return key_code::F;
//        case XK_g:              return key_code::G;
//        case XK_h:              return key_code::H;
//        case XK_i:              return key_code::I;
//        case XK_j:              return key_code::J;
//        case XK_k:              return key_code::K;
//        case XK_l:              return key_code::L;
//        case XK_m:              return key_code::M;
//        case XK_n:              return key_code::N;
//        case XK_o:              return key_code::O;
//        case XK_p:              return key_code::P;
//        case XK_q:              return key_code::Q;
//        case XK_r:              return key_code::R;
        case XK_s:
            return key_code::S;
//        case XK_t:              return key_code::T;
//        case XK_u:              return key_code::U;
        case XK_v:
            return key_code::V;
        case XK_w:
            return key_code::W;
        case XK_x:
            return key_code::X;
        case XK_y:
            return key_code::Y;
        case XK_z:
            return key_code::Z;
//        case XK_1:              return key_code::0;
//        case XK_2:              return key_code::2;
//        case XK_3:              return key_code::3;
//        case XK_4:              return key_code::4;
//        case XK_5:              return key_code::5;
//        case XK_6:              return key_code::6;
//        case XK_7:              return key_code::7;
//        case XK_8:              return key_code::8;
//        case XK_9:              return key_code::9;
//        case XK_0:              return key_code::0;
        case XK_space:
            return key_code::Space;
//        case XK_minus:          return SAPP_KEYCODE_MINUS;
//        case XK_equal:          return SAPP_KEYCODE_EQUAL;
//        case XK_bracketleft:    return SAPP_KEYCODE_LEFT_BRACKET;
//        case XK_bracketright:   return SAPP_KEYCODE_RIGHT_BRACKET;
//        case XK_backslash:      return SAPP_KEYCODE_BACKSLASH;
//        case XK_semicolon:      return SAPP_KEYCODE_SEMICOLON;
//        case XK_apostrophe:     return SAPP_KEYCODE_APOSTROPHE;
//        case XK_grave:          return SAPP_KEYCODE_GRAVE_ACCENT;
//        case XK_comma:          return SAPP_KEYCODE_COMMA;
//        case XK_period:         return SAPP_KEYCODE_PERIOD;
//        case XK_slash:          return SAPP_KEYCODE_SLASH;
//        case XK_less:           return SAPP_KEYCODE_WORLD_1; /* At least in some layouts... */
        default:
            return key_code::unknown;
    }
}

int32_t x11_keysym_to_unicode(KeySym keysym) {
    int min = 0;
    int max = sizeof(_x11_KeySymTab) / sizeof(struct X11_CodePair) - 1;
    int mid;

/* First check for Latin-1 characters (1:1 mapping) */
    if ((keysym >= 0x0020 && keysym <= 0x007e) ||
        (keysym >= 0x00a0 && keysym <= 0x00ff)) {
        return keysym;
    }

/* Also check for directly encoded 24-bit UCS characters */
    if ((keysym & 0xff000000) == 0x01000000) {
        return keysym & 0x00ffffff;
    }

/* Binary search in table */
    while (max >= min) {
        mid = (min + max) / 2;
        if (_x11_KeySymTab[mid].keysym < keysym) {
            min = mid + 1;
        } else if (_x11_KeySymTab[mid].keysym > keysym) {
            max = mid - 1;
        } else {
            return _x11_KeySymTab[mid].ucs;
        }
    }

/* No matching Unicode value found */
    return -1;
}

bool x11_parse_dropped_files_list(const char* src) {
    EK_ASSERT(src);
    return false;
// TODO:
//    EK_ASSERT(_sapp.drop.buffer);
//
//    clear_drop_buffer();
//    _sapp.drop.num_files = 0;

    /*
        src is (potentially percent-encoded) string made of one or multiple paths
        separated by \r\n, each path starting with 'file://'
    */
//    bool err = false;
//    int src_count = 0;
//    char src_chr = 0;
//    char* dst_ptr = _sapp.drop.buffer;
//    const char* dst_end_ptr = dst_ptr + (_sapp.drop.max_path_length - 1); // room for terminating 0
//    while (0 != (src_chr = *src++)) {
//        src_count++;
//        char dst_chr = 0;
//        /* check leading 'file://' */
//        if (src_count <= 7) {
//            if (((src_count == 1) && (src_chr != 'f')) ||
//                ((src_count == 2) && (src_chr != 'i')) ||
//                ((src_count == 3) && (src_chr != 'l')) ||
//                ((src_count == 4) && (src_chr != 'e')) ||
//                ((src_count == 5) && (src_chr != ':')) ||
//                ((src_count == 6) && (src_chr != '/')) ||
//                ((src_count == 7) && (src_chr != '/')))
//            {
//                EK_INFO("sokol_app.h: dropped file URI doesn't start with file://");
//                err = true;
//                break;
//            }
//        }
//        else if (src_chr == '\r') {
//            // skip
//        }
//        else if (src_chr == '\n') {
//            src_chr = 0;
//            src_count = 0;
//            _sapp.drop.num_files++;
//            // too many files is not an error
//            if (_sapp.drop.num_files >= _sapp.drop.max_files) {
//                break;
//            }
//            dst_ptr = _sapp.drop.buffer + _sapp.drop.num_files * _sapp.drop.max_path_length;
//            dst_end_ptr = dst_ptr + (_sapp.drop.max_path_length - 1);
//        }
//        else if ((src_chr == '%') && src[0] && src[1]) {
//            // a percent-encoded byte (most like UTF-8 multibyte sequence)
//            const char digits[3] = { src[0], src[1], 0 };
//            src += 2;
//            dst_chr = (char) strtol(digits, 0, 16);
//        }
//        else {
//            dst_chr = src_chr;
//        }
//        if (dst_chr) {
//            // dst_end_ptr already has adjustment for terminating zero
//            if (dst_ptr < dst_end_ptr) {
//                *dst_ptr++ = dst_chr;
//            }
//            else {
//                EK_INFO("sokol_app.h: dropped file path too long (sapp_desc.max_dropped_file_path_length)");
//                err = true;
//                break;
//            }
//        }
//    }
//    if (err) {
//        clear_drop_buffer();
//        _sapp.drop.num_files = 0;
//        return false;
//    }
//    else {
//        return true;
//    }
}

// XLib manual says keycodes are in the range [8, 255] inclusive.
// https://tronche.com/gui/x/xlib/input/keyboard-encoding.html
static bool x11_keycodes[256];

void x11_process_event(XEvent* event) {
    Bool filtered = XFilterEvent(event, None);
    switch (event->type) {
        case GenericEvent:
            // TODO:
//            if (_sapp.mouse.locked && gAppLinux.x11.xi.available) {
//                if (event->xcookie.extension == gAppLinux.x11.xi.major_opcode) {
//                    if (XGetEventData(gAppLinux.x11.display, &event->xcookie)) {
//                        if (event->xcookie.evtype == XI_RawMotion) {
//                            XIRawEvent* re = (XIRawEvent*) event->xcookie.data;
//                            if (re->valuators.mask_len) {
//                                const double* values = re->raw_values;
//                                if (XIMaskIsSet(re->valuators.mask, 0)) {
//                                    _sapp.mouse.dx = (float) *values;
//                                    values++;
//                                }
//                                if (XIMaskIsSet(re->valuators.mask, 1)) {
//                                    _sapp.mouse.dy = (float) *values;
//                                }
//                                x11_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID, x11_mod(event->xmotion.state));
//                            }
//                        }
//                        XFreeEventData(gAppLinux.x11.display, &event->xcookie);
//                    }
//                }
//            }
            break;
        case FocusOut:
            // TODO:
            /* if focus is lost for any reason, and we're in mouse locked mode, disable mouse lock */
//            if (_sapp.mouse.locked) {
//                x11_lock_mouse(false);
//            }
            break;
        case KeyPress: {

            // TODO:
//            int keycode = (int)event->xkey.keycode;
//            const sapp_keycode key = x11_translate_key(keycode);
//            bool repeat = x11_keycodes[keycode & 0xFF];
//            x11_keycodes[keycode & 0xFF] = true;
//            const uint32_t mods = x11_mod(event->xkey.state);
//            if (key != SAPP_KEYCODE_INVALID) {
//                x11_key_event(SAPP_EVENTTYPE_KEY_DOWN, key, repeat, mods);
//            }
//            KeySym keysym;
//            XLookupString(&event->xkey, NULL, 0, &keysym, NULL);
//            int32_t chr = x11_keysym_to_unicode(keysym);
//            if (chr > 0) {
//                x11_char_event((uint32_t)chr, repeat, mods);
//            }
        }
            break;
        case KeyRelease: {
            // TODO:
//            int keycode = (int)event->xkey.keycode;
//            const sapp_keycode key = x11_translate_key(keycode);
//            x11_keycodes[keycode & 0xFF] = false;
//            if (key != SAPP_KEYCODE_INVALID) {
//                const uint32_t mods = x11_mod(event->xkey.state);
//                x11_key_event(SAPP_EVENTTYPE_KEY_UP, key, false, mods);
//            }
        }
            break;
        case ButtonPress: {
            // TODO:
//            const sapp_mousebutton btn = x11_translate_button(event);
//            const uint32_t mods = x11_mod(event->xbutton.state);
//            if (btn != SAPP_MOUSEBUTTON_INVALID) {
//                x11_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, btn, mods);
//                gAppLinux.x11.mouse_buttons |= (1 << btn);
//            }
//            else {
//                /* might be a scroll event */
//                switch (event->xbutton.button) {
//                    case 4: x11_scroll_event(0.0f, 1.0f, mods); break;
//                    case 5: x11_scroll_event(0.0f, -1.0f, mods); break;
//                    case 6: x11_scroll_event(1.0f, 0.0f, mods); break;
//                    case 7: x11_scroll_event(-1.0f, 0.0f, mods); break;
//                }
//            }
        }
            break;
        case ButtonRelease: {
            // TODO:
//            const sapp_mousebutton btn = x11_translate_button(event);
//            if (btn != SAPP_MOUSEBUTTON_INVALID) {
//                x11_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, btn, x11_mod(event->xbutton.state));
//                gAppLinux.x11.mouse_buttons &= ~(1 << btn);
//            }
        }
            break;
        case EnterNotify:
            /* don't send enter/leave events while mouse button held down */
            if (0 == gAppLinux.x11.mouse_buttons) {
                x11_mouse_event(event_type::mouse_enter, mouse_button::other, x11_mod(event->xcrossing.state));
            }
            break;
        case LeaveNotify:
            if (0 == gAppLinux.x11.mouse_buttons) {
                x11_mouse_event(event_type::mouse_exit, mouse_button::other, x11_mod(event->xcrossing.state));
            }
            break;
        case MotionNotify:
            // TODO:
//            if (!_sapp.mouse.locked) {
//                const float new_x = (float) event->xmotion.x;
//                const float new_y = (float) event->xmotion.y;
//                if (_sapp.mouse.pos_valid) {
//                    _sapp.mouse.dx = new_x - _sapp.mouse.x;
//                    _sapp.mouse.dy = new_y - _sapp.mouse.y;
//                }
//                _sapp.mouse.x = new_x;
//                _sapp.mouse.y = new_y;
//                _sapp.mouse.pos_valid = true;
//                x11_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID, x11_mod(event->xmotion.state));
//            }
            break;
        case ConfigureNotify:
            // TODO:
//            if ((event->xconfigure.width != _sapp.window_width) || (event->xconfigure.height != _sapp.window_height)) {
//                _sapp.window_width = event->xconfigure.width;
//                _sapp.window_height = event->xconfigure.height;
//                _sapp.framebuffer_width = _sapp.window_width;
//                _sapp.framebuffer_height = _sapp.window_height;
//                x11_app_event(SAPP_EVENTTYPE_RESIZED);
//            }
            break;
        case PropertyNotify:
            if (event->xproperty.state == PropertyNewValue) {
                if (event->xproperty.atom == gAppLinux.x11.WM_STATE) {
                    // TODO:
//                    const int state = x11_get_window_state();
//                    if (state != gAppLinux.x11.window_state) {
//                        gAppLinux.x11.window_state = state;
//                        if (state == IconicState) {
//                            x11_app_event(SAPP_EVENTTYPE_ICONIFIED);
//                        }
//                        else if (state == NormalState) {
//                            x11_app_event(SAPP_EVENTTYPE_RESTORED);
//                        }
//                    }
                }
            }
            break;
        case ClientMessage:
            if (filtered) {
                return;
            }
            if (event->xclient.message_type == gAppLinux.x11.WM_PROTOCOLS) {
                const Atom protocol = (Atom) event->xclient.data.l[0];
                if (protocol == gAppLinux.x11.WM_DELETE_WINDOW) {
                    g_app.require_exit = true;
                }
            } else if (event->xclient.message_type == gAppLinux.x11.xdnd.XdndEnter) {
                const bool is_list = 0 != (event->xclient.data.l[1] & 1);
                gAppLinux.x11.xdnd.source = (Window) event->xclient.data.l[0];
                gAppLinux.x11.xdnd.version = event->xclient.data.l[1] >> 24;
                gAppLinux.x11.xdnd.format = None;
                if (gAppLinux.x11.xdnd.version > X11_XDND_VERSION) {
                    return;
                }
                uint32_t count = 0;
                Atom* formats = 0;
                if (is_list) {
                    count = x11_get_window_property(gAppLinux.x11.xdnd.source, gAppLinux.x11.xdnd.XdndTypeList, XA_ATOM,
                                                    (unsigned char**) &formats);
                } else {
                    count = 3;
                    formats = (Atom*) event->xclient.data.l + 2;
                }
                for (uint32_t i = 0; i < count; i++) {
                    if (formats[i] == gAppLinux.x11.xdnd.text_uri_list) {
                        gAppLinux.x11.xdnd.format = gAppLinux.x11.xdnd.text_uri_list;
                        break;
                    }
                }
                if (is_list && formats) {
                    XFree(formats);
                }
            } else if (event->xclient.message_type == gAppLinux.x11.xdnd.XdndDrop) {
                if (gAppLinux.x11.xdnd.version > X11_XDND_VERSION) {
                    return;
                }
                Time time = CurrentTime;
                if (gAppLinux.x11.xdnd.format) {
                    if (gAppLinux.x11.xdnd.version >= 1) {
                        time = (Time) event->xclient.data.l[2];
                    }
                    XConvertSelection(gAppLinux.x11.display,
                                      gAppLinux.x11.xdnd.XdndSelection,
                                      gAppLinux.x11.xdnd.format,
                                      gAppLinux.x11.xdnd.XdndSelection,
                                      gAppLinux.x11.window,
                                      time);
                } else if (gAppLinux.x11.xdnd.version >= 2) {
                    XEvent reply;
                    memset(&reply, 0, sizeof(reply));
                    reply.type = ClientMessage;
                    reply.xclient.window = gAppLinux.x11.window;
                    reply.xclient.message_type = gAppLinux.x11.xdnd.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = (long) gAppLinux.x11.window;
                    reply.xclient.data.l[1] = 0;    // drag was rejected
                    reply.xclient.data.l[2] = None;
                    XSendEvent(gAppLinux.x11.display, gAppLinux.x11.xdnd.source, False, NoEventMask, &reply);
                    XFlush(gAppLinux.x11.display);
                }
            } else if (event->xclient.message_type == gAppLinux.x11.xdnd.XdndPosition) {
                /* drag operation has moved over the window
                   FIXME: we could track the mouse position here, but
                   this isn't implemented on other platforms either so far
                */
                if (gAppLinux.x11.xdnd.version > X11_XDND_VERSION) {
                    return;
                }
                XEvent reply;
                memset(&reply, 0, sizeof(reply));
                reply.type = ClientMessage;
                reply.xclient.window = gAppLinux.x11.xdnd.source;
                reply.xclient.message_type = gAppLinux.x11.xdnd.XdndStatus;
                reply.xclient.format = 32;
                reply.xclient.data.l[0] = (long) gAppLinux.x11.window;
                if (gAppLinux.x11.xdnd.format) {
                    /* reply that we are ready to copy the dragged data */
                    reply.xclient.data.l[1] = 1;    // accept with no rectangle
                    if (gAppLinux.x11.xdnd.version >= 2) {
                        reply.xclient.data.l[4] = (long) gAppLinux.x11.xdnd.XdndActionCopy;
                    }
                }
                XSendEvent(gAppLinux.x11.display, gAppLinux.x11.xdnd.source, False, NoEventMask, &reply);
                XFlush(gAppLinux.x11.display);
            }
            break;
        case SelectionNotify:
            if (event->xselection.property == gAppLinux.x11.xdnd.XdndSelection) {
                char* data = 0;
                uint32_t result = x11_get_window_property(event->xselection.requestor,
                                                          event->xselection.property,
                                                          event->xselection.target,
                                                          (unsigned char**) &data);
                // TODO:
//                if (_sapp.drop.enabled && result) {
//                    if (x11_parse_dropped_files_list(data)) {
//                        if (_sapp_events_enabled()) {
//                            init_event(SAPP_EVENTTYPE_FILES_DROPPED);
//                            call_event(&_sapp.event);
//                        }
//                    }
//                }
                if (gAppLinux.x11.xdnd.version >= 2) {
                    XEvent reply;
                    memset(&reply, 0, sizeof(reply));
                    reply.type = ClientMessage;
                    reply.xclient.window = gAppLinux.x11.window;
                    reply.xclient.message_type = gAppLinux.x11.xdnd.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = (long) gAppLinux.x11.window;
                    reply.xclient.data.l[1] = result;
                    reply.xclient.data.l[2] = (long) gAppLinux.x11.xdnd.XdndActionCopy;
                    XSendEvent(gAppLinux.x11.display, gAppLinux.x11.xdnd.source, False, NoEventMask, &reply);
                    XFlush(gAppLinux.x11.display);
                }
            }
            break;
        case DestroyNotify:
            break;
    }
}

void linux_app_create() {
    /* The following lines are here to trigger a linker error instead of an
        obscure runtime error if the user has forgotten to add -pthread to
        the compiler or linker options. They have no other purpose.
    */
    pthread_attr_t pthread_attr;
    pthread_attr_init(&pthread_attr);
    pthread_attr_destroy(&pthread_attr);

    gAppLinux.x11.window_state = NormalState;

    XInitThreads();
    XrmInitialize();
    gAppLinux.x11.display = XOpenDisplay(NULL);
    if (!gAppLinux.x11.display) {
        ek_fail("XOpenDisplay() failed!\n");
    }
    gAppLinux.x11.screen = DefaultScreen(gAppLinux.x11.display);
    gAppLinux.x11.root = DefaultRootWindow(gAppLinux.x11.display);
    XkbSetDetectableAutoRepeat(gAppLinux.x11.display, true, NULL);
    x11_query_system_dpi();
//    _sapp.dpi_scale = gAppLinux.x11.dpi / 96.0f;
    x11_init_extensions();
    x11_create_hidden_cursor();
    glx_init();
    Visual* visual = 0;
    int depth = 0;
    glx_choose_visual(&visual, &depth);
    x11_create_window(visual, depth);
    glx_create_context();
//    _sapp.valid = true;
    x11_show_window();
//    if (_sapp.fullscreen) {
//        x11_set_fullscreen(true);
//    }
    x11_query_window_size();
//    glx_swapinterval(_sapp.swap_interval);
    XFlush(gAppLinux.x11.display);
}

void linux_app_loop() {
    while (!g_app.require_exit) {
        glx_make_current();
        int count = XPending(gAppLinux.x11.display);
        while (count--) {
            XEvent event;
            XNextEvent(gAppLinux.x11.display, &event);
            x11_process_event(&event);
        }
        dispatch_draw_frame();
        glx_swap_buffers();
        XFlush(gAppLinux.x11.display);
        /* handle quit-requested, either from window or from sapp_request_quit() */
        if (g_app.require_exit) {
            /* give user code a chance to intervene */
            x11_app_event(event_type::app_close);
            /* if user code hasn't intervened, quit the app */
            if (g_app.require_exit) {
                g_app.require_exit = true;
            }
        }
    }
}

void linux_app_shutdown() {
    //    call_cleanup();
    glx_destroy_context();
    x11_destroy_window();
    XCloseDisplay(gAppLinux.x11.display);
//    discard_state();
}