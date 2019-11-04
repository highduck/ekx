#include "platform/Window.h"

#import "AppDelegate.h"

namespace ek {

void Window::updateMouseCursor() {
    NSWindow* wnd = gAppDelegate->nsWindow;
//    [wnd invalidateCursorRectsForView: [wnd contentView]];
    [wnd performSelectorOnMainThread: @selector(invalidateCursorRectsForView:) withObject: [wnd contentView] waitUntilDone: NO];
}

Window::NativeViewContext Window::viewContext() const {
    return gAppDelegate->openGLView;
}

}

