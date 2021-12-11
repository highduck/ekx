#pragma once

#include "GameAppListener.hpp"
#include <ek/ds/Array.hpp>

namespace ek {

class GameAppDispatcher : public GameAppListener {
public:
    Array<GameAppListener*> listeners{};

    ~GameAppDispatcher() override = default;

    void onBeforeFrameBegin() override {
        for (auto* listener : listeners) {
            listener->onBeforeFrameBegin();
        }
    }

    void onPreload() override {
        for (auto* listener : listeners) {
            listener->onPreload();
        }
    }

    void onPreRender() override {
        for (auto* listener : listeners) {
            listener->onPreRender();
        }
    }

    void onRenderOverlay() override {
        for (auto* listener : listeners) {
            listener->onRenderOverlay();
        }
    }

    void onRenderFrame() override {
        for (auto* listener : listeners) {
            listener->onRenderFrame();
        }
    }

    void onUpdate() override {
        for (auto* listener : listeners) {
            listener->onUpdate();
        }
    }

    void onStart() override {
        for (auto* listener : listeners) {
            listener->onStart();
        }
    }

    void onEvent(const ek_app_event& event) override {
        for (auto* listener : listeners) {
            listener->onEvent(event);
        }
    }

    void onPostFrame() override {
        for (auto* listener : listeners) {
            listener->onPostFrame();
        }
    }
};

}