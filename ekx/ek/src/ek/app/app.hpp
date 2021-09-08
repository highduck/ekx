#pragma once

#include <cstdint>
#include <ek/ds/Array.hpp>

#include "events.hpp"

#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

namespace ek {

namespace app {

#if TARGET_OS_MAC

void* getMetalDevice();

const void* getMetalRenderPass();

const void* getMetalDrawable();

#endif

struct WindowConfig final {
    const char* title = nullptr;
    float width = 960;
    float height = 720;
    bool needDepth = false;
    bool webKeepCanvasAspectRatio = false;
    bool allowHighDpi = true;
    int sampleCount = 1;
    int swapInterval = 1;
    uint32_t backgroundColor = 0xFF000000u;
};

struct App final {
    WindowConfig config;

    float windowWidth = 1.0f;
    float windowHeight = 1.0f;
    float drawableWidth = 1.0f;
    float drawableHeight = 1.0f;
    float dpiScale = 1.0f;

    int exitCode = 0;

    MouseCursor cursor = MouseCursor::Parent;
    AppListener* listener = nullptr;

    Array<Event> eventQueue;
    Array<Event> eventQueuePool;

    bool running = false;
    bool preStarted = false;
    bool ready = false;
    bool fullscreen = false;
    bool exitRequired = false;
    bool dirtySize = false;
    bool dirtyCursor = false;
    bool eventQueueLocked = false;

    void updateMouseCursor(MouseCursor cursor_);
};

extern App& g_app;

void process_event(const Event& event);

void dispatch_event(const Event& event);

void dispatch_draw_frame();

void dispatch_init();

void dispatch_device_ready();

void initialize();

void shutdown();

}

void start_application();

#ifndef EK_NO_MAIN

void main();

#endif

}