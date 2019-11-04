#pragma once

#include <scenex/config/ecs.h>

#include <ek/locator.hpp>
#include <baseapp/base_app.hpp>
#include <platform/Application.h>
#include <signal.h>
#include <ek/signals.hpp>
#include <ek/timer.hpp>
#include "app_utils.hpp"

namespace scenex {

class asset_manager_t;

class basic_application : public ek::base_app_t {
public:
    ecs::world w;
    ecs::entity root;
    ecs::entity game;
    ek::float2 base_resolution;

    frame_stats_t stats_{};
    asset_manager_t* asset_manager_ = nullptr;

    basic_application();

    ~basic_application() override;

    void initialize() override;

    void preload() override;

    void onDrawFrame() override;

protected:
    void update_frame(float dt) override;

    void render_frame() override;

    void on_frame_end() override;
};

template<typename T>
void run_app() {
    auto& app = ek::service_locator_instance<basic_application>::init<T>();
    app.initialize();
    app.preload();
    ek::gApp.listen(&app);
}

}


