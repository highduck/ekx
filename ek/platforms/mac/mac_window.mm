#include <platform/window.hpp>

#import "mac_app_delegate.h"

namespace ek {

void window_t::update_mouse_cursor() {
    NSWindow* wnd = g_app_delegate->window;
//    [wnd invalidateCursorRectsForView: [wnd contentView]];
    [wnd performSelectorOnMainThread: @selector(invalidateCursorRectsForView:) withObject: [wnd contentView] waitUntilDone: NO];
}

native_window_context_t window_t::view_context() const {
    return g_app_delegate->gl_view;
}

}

