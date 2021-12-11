#pragma once

#include <ek/app.h>

namespace ek {

class GameAppListener {
public:
    virtual ~GameAppListener() = default;

    virtual void onBeforeFrameBegin() {}

    virtual void onPreload() {}

    // used to render all offscreen passes
    virtual void onPreRender() {}

    virtual void onRenderOverlay() {}

    virtual void onRenderFrame() {}

    virtual void onUpdate() {}

    virtual void onStart() {}

    virtual void onEvent(const ek_app_event&) {}

    virtual void onPostFrame() {}
};

}
