#import <ek/app/app.hpp>
#import "mac_app_delegate.h"

namespace ek {

void application_t::update_mouse_cursor() {
    NSWindow* wnd = g_app_delegate.window;
//    [wnd invalidateCursorRectsForView: [wnd contentView]];
    [wnd performSelectorOnMainThread: @selector(invalidateCursorRectsForView:) withObject: [wnd contentView] waitUntilDone: NO];
}

}

