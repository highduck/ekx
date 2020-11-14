#pragma once

#include <ek/scenex/app/basic_application.hpp>

class DemoApp : public ek::basic_application {
public:
    DemoApp();

    ~DemoApp() override;

    void initialize() override;

    void preload() override;

protected:
    void update_frame(float dt) override;

    void render_frame() override;

    void start_game() override;
};