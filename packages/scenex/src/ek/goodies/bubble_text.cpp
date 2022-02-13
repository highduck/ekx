#include "bubble_text.hpp"

#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/SceneFactory.hpp>

#include <ek/rnd.h>
#include <ekx/app/time_layers.h>
#include <ek/math.h>

namespace ek {

float ease_back5(float t) {
    return ease_back(t, 5);
}

void BubbleText::updateAll() {
    float dt = g_time_layers[TIME_LAYER_HUD].dt;
    const float time_max = 2.0f;
    const float delta_y = -100.0f;

    for (auto e: ecs::view<BubbleText>()) {
        auto& state = ecs::get<BubbleText>(e);

        if (state.delay > 0.0f) {
            state.delay -= dt;
            continue;
        }

        state.time += dt;
        float r = saturate(state.time / time_max);
        float sc = 1.0f;
        float sct;
        vec2_t off = state.offset;
        if (r < 0.5f) {
            sct = r * 2.0f;
            sc = ease_out(ease_p3_out(sct), ease_back5);
            off = state.offset * ease_p3_out(sct);
        }

        auto& transform = ecs::get<Transform2D>(e);
        transform.set_scale(sc);
        vec2_t fly_pos = vec2(0, delta_y * ease_p3_out(r));
        transform.set_position(state.start + off + fly_pos);
        transform.color.scale.a = unorm8_f32_clamped(1.0f - (r * r * r));
        transform.color.offset.a = unorm8_f32_clamped(r * r * r);

        if (state.time >= time_max) {
            destroy_later(e);
        }
    }
}

entity_t BubbleText::create(string_hash_t fontName, vec2_t pos, float delay) {
    auto e = createNode2D();
    auto& c = ecs::add<BubbleText>(e);
    c.delay = delay;
    c.start = pos;
    float spread = 10.0f;
    c.offset.x = random_range_f(-spread, spread);
    TextFormat format{fontName, 32.0f};
    format.setAlignment(Alignment::Center);
    format.setTextColor(COLOR_WHITE);
    format.addShadow(COLOR_BLACK, 4, vec2(0.0f, 1.5f));

    text2d_setup_ex(e, format);

    set_touchable(e, false);
    set_scale(e, vec2(0,0));
    return e;
}

/*
 private function checkOversteps():void {

        var displayWidth:int = Seks.display.width;
		var maxScale:Number = 1.5;
		var halfWidth:int = textField.width * 0.5 * maxScale;
		var global:Point = _producer.layer.localToGlobal(new Point(_startX + _offsetX, _startY));
        var leftOverstep:int = global.x - halfWidth;
        var rightOverstep:int = global.x + halfWidth;

        if (leftOverstep < 0) {
           _startX += Math.abs(leftOverstep);
        }

        if (rightOverstep > displayWidth) {
            _startX -= Math.abs(displayWidth - rightOverstep);
        }
    }
 */
}