#include "mac_input.h"

#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#import <ek/app/app.hpp>
#import <unordered_map>

namespace ek {

void osx_handle_mouse_wheel_scroll(const NSEvent* event, event_t& to_event) {
    to_event.scroll = {event.scrollingDeltaX, event.scrollingDeltaY};
    if (event.hasPreciseScrollingDeltas) {
        to_event.scroll.x *= 0.1;
        to_event.scroll.y *= 0.1;
    }
}

/****** KEYBOARD ****/
static std::unordered_map<uint16_t, key_code> SCAN_CODE_TABLE = {
        {kVK_LeftArrow,         key_code::ArrowLeft},
        {kVK_RightArrow,        key_code::ArrowRight},
        {kVK_DownArrow,         key_code::ArrowDown},
        {kVK_UpArrow,           key_code::ArrowUp},
        {kVK_Escape,            key_code::Escape},
        {kVK_Delete,            key_code::Backspace},
        {kVK_Space,             key_code::Space},
        {kVK_Return,            key_code::Enter},

        {kVK_Tab,               key_code::Tab},
        {kVK_PageUp,            key_code::PageUp},
        {kVK_PageDown,          key_code::PageDown},
        {kVK_Home,              key_code::Home},
        {kVK_End,               key_code::End},
        {kVK_ANSI_KeypadDivide, key_code::Insert},
        {kVK_Delete,            key_code::Delete},
        {kVK_ANSI_A,            key_code::A},
        {kVK_ANSI_C,            key_code::C},
        {kVK_ANSI_V,            key_code::V},
        {kVK_ANSI_X,            key_code::X},
        {kVK_ANSI_Y,            key_code::Y},
        {kVK_ANSI_Z,            key_code::Z},
        {kVK_ANSI_W,            key_code::W},
        {kVK_ANSI_S,            key_code::S},
        {kVK_ANSI_D,            key_code::D}

};

key_code convert_key_code(uint16_t key_code) {
    auto i = SCAN_CODE_TABLE.find(key_code);
    if (i != SCAN_CODE_TABLE.end()) {
        return i->second;
    }
    return key_code::Unknown;
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

}