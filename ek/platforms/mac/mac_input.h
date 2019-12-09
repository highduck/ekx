#pragma once

#import <Cocoa/Cocoa.h>
#import <ek/app/app.hpp>

namespace ek {

void macos_init_common();
void set_view_mouse_cursor(NSView* view);

void handle_mouse_event(NSView* view, NSEvent* event);
bool is_text_event(const NSEvent* event);
bool is_special_character(unichar ch);
void setup_modifiers(NSUInteger flags, event_t& to_event);
key_code convert_key_code(uint16_t key_code);
NSUInteger convert_key_code_to_modifier_mask(uint16_t key_code);

}


