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
#include "impl/analytics_linux.h"
#include "impl/device_linux.h"
#include "impl/resources_linux.h"
#include "impl/sharing_linux.h"
#include "impl/user_prefs_linux.h"

namespace ek {

void start_application() {
    linux_app_run();

    dispatch_init();
    dispatch_device_ready();
}

}

int main(int argc, char* argv[]) {
    g_app.args = {argc, argv};
    ::ek::main();
    return 0;
}