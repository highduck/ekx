#pragma once

#import <Cocoa/Cocoa.h>
#import <ek/app/app.hpp>

namespace ek {

void osx_handle_mouse_wheel_scroll(const NSEvent* event, event_t& to_event);
bool is_text_event(const NSEvent* event);
bool is_special_character(unichar ch);
void setup_modifiers(NSUInteger flags, event_t& to_event);
key_code convert_key_code(uint16_t key_code);
NSUInteger convert_key_code_to_modifier_mask(uint16_t key_code);

}


