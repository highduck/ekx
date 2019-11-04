#pragma once

#import <Cocoa/Cocoa.h>
#import <platform/Application.h>

namespace ek {

void osx_handle_mouse_wheel_scroll(const NSEvent* event, MouseEvent& wheel);
bool is_text_event(const NSEvent* event);
bool is_special_character(unichar ch);
void setup_modifiers(NSUInteger flags, KeyEvent& key);
KeyEvent::Code convert_key_code(uint16_t key_code);
NSUInteger convert_key_code_to_modifier_mask(uint16_t key_code);

}


