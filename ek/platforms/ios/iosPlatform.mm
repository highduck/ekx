#include "AppDelegate.h"
#include <platform/Window.h>

namespace ek {

    void Window::updateMouseCursor() {}
    
    Window::NativeViewContext Window::viewContext() const {
        return (__bridge void*)appDelegate.window.rootViewController.view;
    }
    
}
