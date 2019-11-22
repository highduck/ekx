#include "ios_app_delegate.h"
#include <platform/window.hpp>

namespace ek {

void window_t::update_mouse_cursor() {}

native_window_context_t window_t::view_context() const {
    return (__bridge void*) g_app_delegate.window.rootViewController.view;
}

}
