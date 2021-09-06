#pragma once

#import <Cocoa/Cocoa.h>

// Carbon included for Keyboard Scan Codes
#import <Carbon/Carbon.h>
//#import <Carbon/HIToolbox/Events.h>

#include <ek/debug.hpp>
#include <ek/util/Path.hpp>

@interface NSCursor (HelpCursor)
+ (NSCursor*)_helpCursor;
@end

namespace ek {

using namespace ek::app;

/****** KEYBOARD ****/
KeyCode SCAN_CODE_TABLE[0xFF] = {};

void init_scan_table() {
    SCAN_CODE_TABLE[kVK_LeftArrow] = KeyCode::ArrowLeft;
    SCAN_CODE_TABLE[kVK_LeftArrow] = KeyCode::ArrowLeft;
    SCAN_CODE_TABLE[kVK_RightArrow] = KeyCode::ArrowRight;
    SCAN_CODE_TABLE[kVK_DownArrow] = KeyCode::ArrowDown;
    SCAN_CODE_TABLE[kVK_UpArrow] = KeyCode::ArrowUp;
    SCAN_CODE_TABLE[kVK_Escape] = KeyCode::Escape;
    SCAN_CODE_TABLE[kVK_Delete] = KeyCode::Backspace;
    SCAN_CODE_TABLE[kVK_Space] = KeyCode::Space;
    SCAN_CODE_TABLE[kVK_Return] = KeyCode::Enter;
    SCAN_CODE_TABLE[kVK_Tab] = KeyCode::Tab;
    SCAN_CODE_TABLE[kVK_PageUp] = KeyCode::PageUp;
    SCAN_CODE_TABLE[kVK_PageDown] = KeyCode::PageDown;
    SCAN_CODE_TABLE[kVK_Home] = KeyCode::Home;
    SCAN_CODE_TABLE[kVK_End] = KeyCode::End;
    SCAN_CODE_TABLE[kVK_ANSI_KeypadDivide] = KeyCode::Insert;
    SCAN_CODE_TABLE[kVK_ForwardDelete] = KeyCode::Delete;
    SCAN_CODE_TABLE[kVK_ANSI_A] = KeyCode::A;
    SCAN_CODE_TABLE[kVK_ANSI_C] = KeyCode::C;
    SCAN_CODE_TABLE[kVK_ANSI_V] = KeyCode::V;
    SCAN_CODE_TABLE[kVK_ANSI_X] = KeyCode::X;
    SCAN_CODE_TABLE[kVK_ANSI_Y] = KeyCode::Y;
    SCAN_CODE_TABLE[kVK_ANSI_Z] = KeyCode::Z;
    SCAN_CODE_TABLE[kVK_ANSI_W] = KeyCode::W;
    SCAN_CODE_TABLE[kVK_ANSI_S] = KeyCode::S;
    SCAN_CODE_TABLE[kVK_ANSI_D] = KeyCode::D;
}

KeyCode convertKeyCode(uint16_t code) {
    if (code < 0xFF) {
        return SCAN_CODE_TABLE[code];
    }
    return KeyCode::Unknown;
}

KeyModifier convertKeyModifiers(NSUInteger flags) {
    int mod = 0;
    if (flags & NSEventModifierFlagOption) mod |= (int) app::KeyModifier::Alt;
    if (flags & NSEventModifierFlagShift) mod |= (int) app::KeyModifier::Shift;
    if (flags & NSEventModifierFlagControl) mod |= (int) app::KeyModifier::Control;
    if (flags & NSEventModifierFlagCommand) mod |= (int) app::KeyModifier::Super;
    return (app::KeyModifier) mod;
}

bool is_special_character(unichar ch) {

    if (ch == kDeleteCharCode) {
        return true;
    }
    // From QT:
    // ignore text for the U+F700-U+F8FF range. This is used by Cocoa when
    // delivering function keys (e.g. arrow keys, backspace, F1-F35, etc.)
    return ch >= 0xF700u && ch <= 0xF8FFu;
}

bool is_text_event(const NSEvent* event) {
    if (event.characters && event.characters.length > 0) {
        return !is_special_character([event.characters characterAtIndex:0]);
    }
    return false;
}

NSUInteger convert_key_code_to_modifier_mask(uint16_t key_code) {
    NSUInteger mask = 0u;
    switch (key_code) {
        case kVK_Control:
        case kVK_RightControl:
            mask = NSEventModifierFlagControl;
            break;
        case kVK_Command:
        case kVK_RightCommand:
            mask = NSEventModifierFlagCommand;
            break;
        case kVK_Shift:
        case kVK_RightShift:
            mask = NSEventModifierFlagShift;
            break;
        case kVK_Option:
        case kVK_RightOption:
            mask = NSEventModifierFlagOption;
            break;
        default:
            break;
    }
    return mask;
}

void set_view_mouse_cursor(NSView* view) {
    NSCursor* cursor = nil;

    switch (g_app.cursor) {
        case MouseCursor::Button:
            cursor = NSCursor.pointingHandCursor;
            break;
        case MouseCursor::Help:
            cursor = NSCursor._helpCursor;
            break;
        case MouseCursor::Arrow:
        case MouseCursor::Parent:
            cursor = NSCursor.arrowCursor;
            break;
    }

    if (cursor) {
        [view addCursorRect:view.bounds cursor:cursor];
        [cursor set];
    }
}

void macos_init_common() {
    EK_TRACE << "app macOS: init commons";
    init_scan_table();
}

void handleMouseWheelScroll(const NSEvent* event, MouseEvent& mouseEvent) {
    mouseEvent.scrollX = static_cast<float>(event.scrollingDeltaX);
    mouseEvent.scrollY = static_cast<float>(event.scrollingDeltaY);
    if (event.hasPreciseScrollingDeltas) {
        mouseEvent.scrollX *= 0.1f;
        mouseEvent.scrollY *= 0.1f;
    }
}

void handleMouseEvent(NSView* view, NSEvent* event) {
    const auto location = [view convertPoint:event.locationInWindow fromView:nil];
    const auto scale = view.window.backingScaleFactor;

    Event::Type type;
    MouseEvent ev{};
    ev.button = MouseButton::Other;
    ev.x = static_cast<float>(location.x * scale);
    ev.y = static_cast<float>(location.y * scale);
    ev.scrollX = 0.0f;
    ev.scrollY = 0.0f;

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeLeftMouseUp:
            ev.button = MouseButton::Left;
            break;
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeRightMouseUp:
            ev.button = MouseButton::Right;
            break;
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeOtherMouseUp:
            ev.button = MouseButton::Other;
            break;
        default:
            break;
    }

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
            type = Event::MouseDown;
            break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
            type = Event::MouseUp;
            break;
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeMouseMoved:
            type = Event::MouseMove;
            break;
        case NSEventTypeMouseEntered:
            type = Event::MouseEnter;
            break;
        case NSEventTypeMouseExited:
            type = Event::MouseExit;
            break;
        case NSEventTypeScrollWheel:
            type = Event::MouseScroll;
            handleMouseWheelScroll(event, ev);
            break;
        default:
            return;
    }
    dispatch_event(Event::Mouse(type, ev));
}

}