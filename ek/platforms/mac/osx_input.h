#pragma once

#import <Cocoa/Cocoa.h>
#import <platform/application.hpp>

namespace ek {

void osx_handle_mouse_wheel_scroll(const NSEvent* event, mouse_event_t& wheel);
bool is_text_event(const NSEvent* event);
bool is_special_character(unichar ch);
void setup_modifiers(NSUInteger flags, key_event_t& key);
key_code convert_key_code(uint16_t key_code);
NSUInteger convert_key_code_to_modifier_mask(uint16_t key_code);

}


