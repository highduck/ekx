#include "bubble_text.hpp"

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

    for (auto e: ecs::view_backward<BubbleText>()) {
        auto& state = e.get<BubbleText>();

        if (state.delay > 0.0f) {
            state.delay -= dt;
            continue;
        }

        state.time += dt;

        if (state.time >= time_max) {
            destroyNode(e);
            continue;
        }

        float r = saturate(state.time / time_max);
        float sc = 1.0f;
        float sct;
        vec2_t off = state.offset;
        if (r < 0.5f) {
            sct = r * 2.0f;
            sc = ease_out(ease_p3_out(sct), ease_back5);
            off = state.offset * ease_p3_out(sct);
        }

        auto& transform = e.get<Transform2D>();
        transform.setScale(sc);
        vec2_t fly_pos = vec2(0, delta_y * ease_p3_out(r));
        transform.setPosition(state.start + off + fly_pos);
        transform.color.scale.a = unorm8_f32_clamped(1.0f - (r * r * r));
        transform.color.offset.a = unorm8_f32_clamped(r * r * r);
    }
}

ecs::EntityApi BubbleText::create(string_hash_t fontName, const String& text, vec2_t pos, float delay) {
    auto e = createNode2D();
    auto& c = e.assign<BubbleText>();
    c.delay = delay;
    c.start = pos;
    float spread = 10.0f;
    c.offset.x = random_range_f(-spread, spread);
    TextFormat format{fontName, 32.0f};
    format.setAlignment(Alignment::Center);
    format.setTextColor(COLOR_WHITE);
    format.addShadow(COLOR_BLACK, 4, vec2(0.0f, 1.5f));

    text2d_setup_ex(e.index, text, format);

    setTouchable(e, false);
    setScale(e, {});
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