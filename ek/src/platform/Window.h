#pragma once

#include "window_creation_config.h"

#include <cstdint>
#include <string>

namespace ek {

class Window final {
public:

    enum class Cursor {
        Auto,
        Button,
        Help
    };

    struct Size final {
        uint32_t width = 1;
        uint32_t height = 1;
    };

    Size windowSize{};
    Size backBufferSize{};
    float scaleFactor = 1.0f;

    typedef void* NativeViewContext;

    NativeViewContext viewContext() const;

    Cursor cursor() const {
        return mCursor;
    }

    void setCursor(Cursor cur) {
        mCursor = cur;
        updateMouseCursor();
    }

    bool sizeChanged = false;

    window_creation_config_t creation_config;

private:

    void updateMouseCursor();

    Cursor mCursor = Cursor::Auto;

};

using MouseCursor = Window::Cursor;

extern Window gWindow;

}


