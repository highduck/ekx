#include "main_flow.hpp"

#include <ek/util/locator.hpp>

#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/goodies/Shake.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/goodies/PopupManager.hpp>
#include <ek/timers.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/Canvas.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/goodies/GameScreen.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <Tracy.hpp>

namespace ek {

using namespace ecs;

void scene_pre_update(entity root, float dt) {
    ZoneScoped;

    resolve<InteractionSystem>().process();

    TimeLayer::updateTimers(dt);

    resolve<AudioManager>().update(dt);

    {
        auto* screenManager = try_resolve<GameScreenManager>();
        if (screenManager) {
            screenManager->update();
        }
    }
    Canvas::updateAll();
    LayoutRect::updateAll();
    Tween::updateAll();
    Shake::updateAll();

    BubbleText::updateAll();
    PopupManager::updateAll();

    updateScripts();
    Button::updateAll();
    MovieClip::updateAll();
}

void scene_post_update(ecs::entity root) {
    ZoneScoped;

    DestroyTimer::updateAll();

    updateWorldTransformAll(&ecs::the_world, root);

    Trail2D::updateAll();
    update_emitters();
    update_particles();
    Camera2D::updateQueue();
}

static volatile uint32_t A1 = 0;

__attribute__((noinline))
void iterate_size_t(size_t n) {
    for(size_t i = 0; i < n; ++i) {
        ++A1;
    }
};

static volatile uint32_t A2 = 0;
__attribute__((noinline))
void iterate_int32(uint32_t n) {
    for(uint32_t i = 0; i < n; ++i) {
        ++A2;
    }
};

static volatile uint32_t A3 = 0;
__attribute__((noinline))
void iterate_int16(uint16_t n) {
    for(uint16_t i = 0; i < n; ++i) {
        ++A3;
    }
};

static volatile uint32_t A4 = 0;
__attribute__((noinline))
void iterate_int16_fast(uint_fast16_t n) {
    for(uint_fast16_t i = 0; i < n; ++i) {
        ++A4;
    }
};

void scene_render(ecs::entity root) {
    ZoneScoped;

    Camera2D::render();
//    drawScene2D(root);
    //drawSceneGizmos(root);


    {
        const size_t N = 50000;
        iterate_size_t(N);
    }
    {
        const uint32_t N = 50000;
        iterate_int32(N);
    }
    {
        const uint16_t N = 50000;
        iterate_int16(N);
    }

    {
        const uint16_t N = 50000;
        iterate_int16_fast(N);
    }

    for (auto& it : Res<DynamicAtlas>::map()) {
        auto* atlas = const_cast<DynamicAtlas*>(it.second->content);
        if (atlas) {
            atlas->invalidate();
        }
    }

//    for (auto& it : Res<DynamicAtlas>::map()) {
//        auto* atlas = const_cast<DynamicAtlas*>(it.second->content);
//        if (atlas) {
//            atlas->reset();
//        }
//    }
}

}


