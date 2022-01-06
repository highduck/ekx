#pragma once

#include <ek/log.h>
#include "../intern.h"

#import <Cocoa/Cocoa.h>

// Carbon included for Keyboard Scan Codes
#import <Carbon/Carbon.h>
//#import <Carbon/HIToolbox/Events.h>

@interface NSCursor (HelpCursor)
+ (NSCursor*)_helpCursor;
@end

/****** KEYBOARD ****/
static ek_key_code ek_scan_code_table[0xFF];

static void initScanCodeTableApple() {
    ek_scan_code_table[kVK_LeftArrow] = EK_KEYCODE_ARROW_LEFT;
    ek_scan_code_table[kVK_RightArrow] = EK_KEYCODE_ARROW_RIGHT;
    ek_scan_code_table[kVK_DownArrow] = EK_KEYCODE_ARROW_DOWN;
    ek_scan_code_table[kVK_UpArrow] = EK_KEYCODE_ARROW_UP;
    ek_scan_code_table[kVK_Escape] = EK_KEYCODE_ESCAPE;
    ek_scan_code_table[kVK_Delete] = EK_KEYCODE_BACKSPACE;
    ek_scan_code_table[kVK_Space] = EK_KEYCODE_SPACE;
    ek_scan_code_table[kVK_Return] = EK_KEYCODE_ENTER;
    ek_scan_code_table[kVK_Tab] = EK_KEYCODE_TAB;
    ek_scan_code_table[kVK_PageUp] = EK_KEYCODE_PAGEUP;
    ek_scan_code_table[kVK_PageDown] = EK_KEYCODE_PAGEDOWN;
    ek_scan_code_table[kVK_Home] = EK_KEYCODE_HOME;
    ek_scan_code_table[kVK_End] = EK_KEYCODE_END;
    ek_scan_code_table[kVK_ANSI_KeypadDivide] = EK_KEYCODE_INSERT;
    ek_scan_code_table[kVK_ForwardDelete] = EK_KEYCODE_DELETE;
    ek_scan_code_table[kVK_ANSI_A] = EK_KEYCODE_A;
    ek_scan_code_table[kVK_ANSI_C] = EK_KEYCODE_C;
    ek_scan_code_table[kVK_ANSI_V] = EK_KEYCODE_V;
    ek_scan_code_table[kVK_ANSI_X] = EK_KEYCODE_X;
    ek_scan_code_table[kVK_ANSI_Y] = EK_KEYCODE_Y;
    ek_scan_code_table[kVK_ANSI_Z] = EK_KEYCODE_Z;
    ek_scan_code_table[kVK_ANSI_W] = EK_KEYCODE_W;
    ek_scan_code_table[kVK_ANSI_S] = EK_KEYCODE_S;
    ek_scan_code_table[kVK_ANSI_D] = EK_KEYCODE_D;
}

static ek_key_code toKeyCode(uint16_t code) {
    if (code < 0xFF) {
        return ek_scan_code_table[code];
    }
    return EK_KEYCODE_UNKNOWN;
}

static ek_key_mod toKeyModifiers(NSUInteger flags) {
    unsigned mod = 0;
    if (flags & NSEventModifierFlagOption) mod |= (unsigned) EK_KEY_MOD_ALT;
    if (flags & NSEventModifierFlagShift) mod |= (unsigned) EK_KEY_MOD_SHIFT;
    if (flags & NSEventModifierFlagControl) mod |= (unsigned) EK_KEY_MOD_CONTROL;
    if (flags & NSEventModifierFlagCommand) mod |= (unsigned) EK_KEY_MOD_SUPER;
    return (ek_key_mod) mod;
}

static bool isSpecialCharacter(unichar ch) {
    if (ch == kDeleteCharCode) {
        return true;
    }
    // From QT:
    // ignore text for the U+F700-U+F8FF range. This is used by Cocoa when
    // delivering function keys (e.g. arrow keys, backspace, F1-F35, etc.)
    return ch >= 0xF700u && ch <= 0xF8FFu;
}

static bool isTextEvent(const NSEvent* event) {
    if (event.characters && event.characters.length > 0) {
        return !isSpecialCharacter([event.characters characterAtIndex:0]);
    }
    return false;
}

static NSUInteger extractKeyModifiers(uint16_t code) {
    NSUInteger mask = 0u;
    switch (code) {
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

static void setViewMouseCursor(NSView* view) {
    NSCursor* cursor = nil;

    switch (ek_app.cursor) {
        case EK_MOUSE_CURSOR_BUTTON:
            cursor = NSCursor.pointingHandCursor;
            break;
        case EK_MOUSE_CURSOR_HELP:
            cursor = NSCursor._helpCursor;
            break;
        case EK_MOUSE_CURSOR_ARROW:
        case EK_MOUSE_CURSOR_PARENT:
            cursor = NSCursor.arrowCursor;
            break;
        default:
            __builtin_unreachable();
    }

    if (cursor) {
        [view addCursorRect:view.bounds cursor:cursor];
        [cursor set];
    }
}

static void handleWheelEvent(NSView* view, const NSEvent* event) {
    (void)sizeof(view);

    CGFloat x = event.scrollingDeltaX;
    CGFloat y = event.scrollingDeltaY;
    if (event.hasPreciseScrollingDeltas) {
        x /= 10;
        y /= 10;
    }
    ek_app__process_event((ek_app_event) {.wheel = {
            .type = EK_APP_EVENT_WHEEL,
            .x = (float) x,
            .y = (float) y
    }});
}

static ek_mouse_button ek_apple__convert_mouse_button(NSEventType type) {
    switch ((uint32_t)type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeLeftMouseUp:
            return EK_MOUSE_BUTTON_LEFT;
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeRightMouseUp:
            return EK_MOUSE_BUTTON_RIGHT;
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeOtherMouseUp:
        default:
            return EK_MOUSE_BUTTON_OTHER;
    }
}

static void handleMouseEvent(NSView* view, const NSEvent* event) {
    const CGPoint location = [view convertPoint:event.locationInWindow fromView:nil];
    const CGFloat scale = view.window.backingScaleFactor;

    ek_app_event ev = {.mouse = {
            .button = ek_apple__convert_mouse_button(event.type),
            .x = (float)(location.x * scale),
            .y = (float)(location.y * scale),
    }};

    switch ((uint32_t)event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
            ev.type = EK_APP_EVENT_MOUSE_DOWN;
            break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
            ev.type = EK_APP_EVENT_MOUSE_UP;
            break;
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeMouseMoved:
            ev.type = EK_APP_EVENT_MOUSE_MOVE;
            break;
        case NSEventTypeMouseEntered:
            ev.type = EK_APP_EVENT_MOUSE_ENTER;
            break;
        case NSEventTypeMouseExited:
            ev.type = EK_APP_EVENT_MOUSE_EXIT;
            break;
        default:
            return;
    }
    ek_app__process_event(ev);
}

