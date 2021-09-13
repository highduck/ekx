#pragma once

#include <cstdint>

#include "events.hpp"

namespace ek::app {

struct AppConfig final {
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

struct AppContext {
    AppConfig config;

    float windowWidth = 1.0f;
    float windowHeight = 1.0f;
    float drawableWidth = 1.0f;
    float drawableHeight = 1.0f;
    float dpiScale = 1.0f;

    int exitCode = 0;

    MouseCursor cursor = MouseCursor::Parent;
    AppListener* listener = nullptr;

    bool initialized = false;
    bool ready = false;
    bool running = false;

    bool fullscreen = false;
    bool exitRequired = false;
    bool dirtySize = false;
    bool dirtyCursor = false;
};

inline AppContext g_app{};

void start();

void quit(int status);

void cancelQuit();

void setMouseCursor(MouseCursor cursor_);

/**
 * Default lang on device during application creation (currently default for user on the device)
 * @return pointer to language code string or nullptr in case of error or not supported
 */
const char* getPreferredLang();

/**
 * Safe area margins left-top-right-bottom
 * @return pointer to 4 floats array, nullptr in case if not available
 */
const float* getScreenInsets();

void vibrate(int durationMillis);

const char* getSystemFontPath(const char* fontName);

#ifndef EK_NO_MAIN

void main();

#endif

// internal functions
void processEvent(const Event& event);

void processFrame();

void notifyInit();

void notifyReady();

}