#pragma once

#include <ek/app.h>
#include <ek/math.h>

#include <ecx/ecx_fwd.hpp>
#include <ek/hash.h>

#define POINTER_EVENT_DOWN H("pointer_down")
#define POINTER_EVENT_UP H("pointer_up")
#define POINTER_EVENT_OVER H("pointer_over")
#define POINTER_EVENT_OUT H("pointer_out")
#define POINTER_EVENT_TAP H("pointer_tap")

namespace ek {

struct Interactive {
    ek_mouse_cursor cursor = EK_MOUSE_CURSOR_PARENT;

    // TODO: Interactive::pointer is useless
    // global world space pointer, because of that maybe we don't dispatch this coordinate into components,
    // just use global pointer from Interaction System
    vec2_t pointer = {};
    entity_t camera = NULL_ENTITY;

    //bool enabled = true;
    // todo: check bubble was false by default
    bool bubble = true;
    // responsive state
    bool over = false;
    bool pushed = false;

    void handle(entity_t e, string_hash_t type);
};

}