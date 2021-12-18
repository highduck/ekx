#pragma once

#include <ek/time.h>

namespace ek {

template<typename Fn>
inline ek_timer_callback timer_func(Fn&& fn) {
    ek_timer_callback callback;
    if constexpr(sizeof(Fn) > sizeof(void*)) {
        callback.userdata = new Fn(fn);
        callback.action = [](void* pf) {
            ((Fn*) pf)->operator()();
        };
        callback.cleanup = [](void* pf) {
            delete (Fn*) pf;
        };
        //log_debug("created HEAP callback!");
    } else {
        new(&callback.userdata)Fn(fn);
        callback.action = [](void* pf) {
            ((Fn*) (&pf))->operator()();
        };
        callback.cleanup = [](void* pf) {
            ((Fn*) (&pf))->~Fn();
        };
        //log_debug("created INLINE callback!");
    }
    return callback;
}

}