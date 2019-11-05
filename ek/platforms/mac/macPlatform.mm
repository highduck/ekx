#include "platform/window.hpp"

#import "AppDelegate.h"

namespace ek {

void window_t::update_mouse_cursor() {
    NSWindow* wnd = gAppDelegate->nsWindow;
//    [wnd invalidateCursorRectsForView: [wnd contentView]];
    [wnd performSelectorOnMainThread: @selector(invalidateCursorRectsForView:) withObject: [wnd contentView] waitUntilDone: NO];
}

native_window_context_t window_t::view_context() const {
    return gAppDelegate->openGLView;
}

}

