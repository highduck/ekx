#include "osx_input.h"

#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#import <platform/Application.h>
#import <unordered_map>

namespace ek {

void osx_handle_mouse_wheel_scroll(const NSEvent* event, MouseEvent& wheel) {
    double dx = 0.0;
    double dy = 0.0;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6) {
        dx = event.scrollingDeltaX;
        dy = event.scrollingDeltaY;
        if (event.hasPreciseScrollingDeltas) {
            dx *= 0.1;
            dy *= 0.1;
        }
    } else
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
    {
        dx = event.deltaX;
        dy = event.deltaY;
    }
    wheel.scrollX = static_cast<float>(dx);
    wheel.scrollY = static_cast<float>(dy);
}



/****** KEYBOARD ****/
static std::unordered_map<uint16_t, KeyEvent::Code> SCAN_CODE_TABLE = {
        {kVK_LeftArrow,         KeyEvent::Code::ArrowLeft},
        {kVK_RightArrow,        KeyEvent::Code::ArrowRight},
        {kVK_DownArrow,         KeyEvent::Code::ArrowDown},
        {kVK_UpArrow,           KeyEvent::Code::ArrowUp},
        {kVK_Escape,            KeyEvent::Code::Escape},
        {kVK_Delete,            KeyEvent::Code::Backspace},
        {kVK_Space,             KeyEvent::Code::Space},
        {kVK_Return,            KeyEvent::Code::Enter},

        {kVK_Tab,               KeyEvent::Code::Tab},
        {kVK_PageUp,            KeyEvent::Code::PageUp},
        {kVK_PageDown,          KeyEvent::Code::PageDown},
        {kVK_Home,              KeyEvent::Code::Home},
        {kVK_End,               KeyEvent::Code::End},
        {kVK_ANSI_KeypadDivide, KeyEvent::Code::Insert},
        {kVK_Delete,            KeyEvent::Code::Delete},
        {kVK_ANSI_A,            KeyEvent::Code::A},
        {kVK_ANSI_C,            KeyEvent::Code::C},
        {kVK_ANSI_V,            KeyEvent::Code::V},
        {kVK_ANSI_X,            KeyEvent::Code::X},
        {kVK_ANSI_Y,            KeyEvent::Code::Y},
        {kVK_ANSI_Z,            KeyEvent::Code::Z},
        {kVK_ANSI_W,            KeyEvent::Code::W},
        {kVK_ANSI_S,            KeyEvent::Code::S},
        {kVK_ANSI_D,            KeyEvent::Code::D}

};

KeyEvent::Code convert_key_code(uint16_t key_code) {
    auto i = SCAN_CODE_TABLE.find(key_code);
    if (i != SCAN_CODE_TABLE.end()) {
        return i->second;
    }
    return KeyEvent::Code::Unknown;
}

void setup_modifiers(NSUInteger flags, KeyEvent& key) {
    key.alt = (flags & NSEventModifierFlagOption) != 0;
    key.shift = (flags & NSEventModifierFlagShift) != 0;
    key.ctrl = (flags & NSEventModifierFlagControl) != 0;
    key.super = (flags & NSEventModifierFlagCommand) != 0;
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
        return !is_special_character([event.characters characterAtIndex: 0]);
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

}