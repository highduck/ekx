#include "AppDelegate.h"
#include <platform/window.hpp>

namespace ek {

    void window_t::update_mouse_cursor() {}

    native_window_context_t window_t::view_context() const {
        return (__bridge void*)appDelegate.window.rootViewController.view;
    }
    
}
