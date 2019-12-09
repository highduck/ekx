#pragma once

#include <scenex/config/ecs.h>

#include <ek/util/locator.hpp>
#include <baseapp/base_app.hpp>
#include <ek/app/app.hpp>
#include <ek/util/signals.hpp>
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

    void on_draw_frame() override;

protected:
    void update_frame(float dt) override;

    void render_frame() override;

    void on_frame_end() override;

    virtual void preload_root_assets_pack();

    virtual void start_game();

    bool started_ = false;
};

template<typename T>
void run_app() {
    auto& app = ek::service_locator_instance<basic_application>::init<T>();
    app.initialize();
    app.preload();
    ek::g_app.listen(&app);
}

}


