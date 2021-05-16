#pragma once

#import <Cocoa/Cocoa.h>

// Carbon included for Keyboard Scan Codes
#import <Carbon/Carbon.h>
//#import <Carbon/HIToolbox/Events.h>

#include <ek/debug.hpp>
#include <ek/util/path.hpp>

@interface NSCursor (HelpCursor)
+ (NSCursor*)_helpCursor;
@end

namespace ek {

using namespace ek::app;

/****** KEYBOARD ****/
key_code SCAN_CODE_TABLE[0xFF] = {};

void init_scan_table() {
    SCAN_CODE_TABLE[kVK_LeftArrow] = key_code::ArrowLeft;
    SCAN_CODE_TABLE[kVK_LeftArrow] = key_code::ArrowLeft;
    SCAN_CODE_TABLE[kVK_RightArrow] = key_code::ArrowRight;
    SCAN_CODE_TABLE[kVK_DownArrow] = key_code::ArrowDown;
    SCAN_CODE_TABLE[kVK_UpArrow] = key_code::ArrowUp;
    SCAN_CODE_TABLE[kVK_Escape] = key_code::Escape;
    SCAN_CODE_TABLE[kVK_Delete] = key_code::Backspace;
    SCAN_CODE_TABLE[kVK_Space] = key_code::Space;
    SCAN_CODE_TABLE[kVK_Return] = key_code::Enter;
    SCAN_CODE_TABLE[kVK_Tab] = key_code::Tab;
    SCAN_CODE_TABLE[kVK_PageUp] = key_code::PageUp;
    SCAN_CODE_TABLE[kVK_PageDown] = key_code::PageDown;
    SCAN_CODE_TABLE[kVK_Home] = key_code::Home;
    SCAN_CODE_TABLE[kVK_End] = key_code::End;
    SCAN_CODE_TABLE[kVK_ANSI_KeypadDivide] = key_code::Insert;
    SCAN_CODE_TABLE[kVK_ForwardDelete] = key_code::Delete;
    SCAN_CODE_TABLE[kVK_ANSI_A] = key_code::A;
    SCAN_CODE_TABLE[kVK_ANSI_C] = key_code::C;
    SCAN_CODE_TABLE[kVK_ANSI_V] = key_code::V;
    SCAN_CODE_TABLE[kVK_ANSI_X] = key_code::X;
    SCAN_CODE_TABLE[kVK_ANSI_Y] = key_code::Y;
    SCAN_CODE_TABLE[kVK_ANSI_Z] = key_code::Z;
    SCAN_CODE_TABLE[kVK_ANSI_W] = key_code::W;
    SCAN_CODE_TABLE[kVK_ANSI_S] = key_code::S;
    SCAN_CODE_TABLE[kVK_ANSI_D] = key_code::D;
}

key_code convert_key_code(uint16_t key_code) {
    if (key_code < 0xFF) {
        return SCAN_CODE_TABLE[key_code];
    }
    return key_code::unknown;
}

void setup_modifiers(NSUInteger flags, event_t& to_event) {
    to_event.alt = (flags & NSEventModifierFlagOption) != 0;
    to_event.shift = (flags & NSEventModifierFlagShift) != 0;
    to_event.ctrl = (flags & NSEventModifierFlagControl) != 0;
    to_event.super = (flags & NSEventModifierFlagCommand) != 0;
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
        case mouse_cursor::button:
            cursor = NSCursor.pointingHandCursor;
            break;
        case mouse_cursor::help:
            cursor = NSCursor._helpCursor;
            break;
        case mouse_cursor::arrow:
        case mouse_cursor::parent:
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

void handle_mouse_wheel_scroll(const NSEvent* event, event_t& to_event) {
    to_event.scroll.x = static_cast<float>(event.scrollingDeltaX);
    to_event.scroll.y = static_cast<float>(event.scrollingDeltaY);
    if (event.hasPreciseScrollingDeltas) {
        to_event.scroll.x *= 0.1f;
        to_event.scroll.y *= 0.1f;
    }
}

void handle_mouse_event(NSView* view, NSEvent* event) {
    const auto location = [view convertPoint:event.locationInWindow fromView:nil];
    const auto scale = view.window.backingScaleFactor;
    event_t ev{};
    ev.pos.x = static_cast<float>(location.x * scale);
    ev.pos.y = static_cast<float>(location.y * scale);

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeLeftMouseUp:
            ev.button = mouse_button::left;
            break;
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeRightMouseUp:
            ev.button = mouse_button::right;
            break;
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeOtherMouseUp:
            ev.button = mouse_button::other;
            break;
        default:
            break;
    }

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
            ev.type = event_type::mouse_down;
            break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
            ev.type = event_type::mouse_up;
            break;
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeMouseMoved:
            ev.type = event_type::mouse_move;
            break;
        case NSEventTypeMouseEntered:
            ev.type = event_type::mouse_enter;
            break;
        case NSEventTypeMouseExited:
            ev.type = event_type::mouse_exit;
            break;
        case NSEventTypeScrollWheel:
            ev.type = event_type::mouse_scroll;
            handle_mouse_wheel_scroll(event, ev);
            break;
        default:
            break;
    }
    dispatch_event(ev);
}

}