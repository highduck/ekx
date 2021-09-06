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

#include <ek/Arguments.hpp>
#include "impl/window_linux.h"
#include "impl/analytics_linux.h"
#include "impl/device_linux.h"
#include "impl/resources_linux.h"
#include "impl/sharing_linux.h"
#include "impl/user_prefs_linux.h"

namespace ek {

void start_application() {
    app::dispatch_init();
    linux_app_create();
    app::dispatch_device_ready();

    linux_app_loop();
    linux_app_shutdown();
}

}

int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::main();
    return 0;
}