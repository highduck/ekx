#pragma once

#include <ek/app/app.hpp>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>
#include <ek/log.h>
#include <ek/assert.h>
#include <sokol_gfx.h>

#ifdef TRACY_ENABLE

#include <Tracy.hpp>

#endif

namespace ek {

class RootAppListener : public app::AppListener {
public:

    ~RootAppListener() override = default;

    void onFrame() override {
#ifdef TRACY_ENABLE
        // Tracy integration
        tracy::SetThreadName("Render");
        FrameMark;
#endif
        log_tick();
        dispatchTimers();
    }

    void onEvent(const app::Event& event) override {
        using ek::app::EventType;
        if (event.type == EventType::Pause) {
            auph::pause();
        } else if (event.type == EventType::Resume) {
            auph::resume();
        } else if (event.type == EventType::Close) {
            auph::shutdown();
            sg_shutdown();
        }
    }

};

}