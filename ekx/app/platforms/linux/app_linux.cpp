#define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xmd.h> /* CARD32 */
#include <dlfcn.h> /* dlopen, dlsym, dlclose */
#include <limits.h> /* LONG_MAX */
#include <pthread.h> /* only used a linker-guard, search for _sapp_linux_run() and see first comment */

#include "impl/window_linux.h"
#include "impl/device_linux.h"
#include "impl/resources_linux.h"
#include "impl/sharing_linux.h"
#include "impl/user_prefs_linux.h"

int main(int argc, char* argv[]) {
    using namespace ek::app;
    g_app.argc = argc;
    g_app.argv = argv;

    ::ek::app::main();
    if (!g_app.exitRequired) {
        linux_app_create();
        notifyReady();

        linux_app_loop();

        linux_app_shutdown();
    }
    return g_app.exitCode;
}