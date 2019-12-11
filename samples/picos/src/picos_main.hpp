#pragma once

#ifdef EK_EDITOR

#include <ek/editor/editor_app.hpp>

using base_app_type = ::ek::editor_app_t;

#else

#include <ek/scenex/ek/basic_application.hpp>

using base_app_type = ::ek::basic_application;

#endif

class PikoApp : public base_app_type {
public:
    PikoApp();

    ~PikoApp() override;

    void initialize() override;

    void preload() override;

protected:
    void update_frame(float dt) override;

    void render_frame() override;

    void start_game() override;
};