#include "bubble_text.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/math/easing.hpp>
#include <ek/math/rand.hpp>

namespace ek {

void update_bubble_text(float dt) {
    const float time_max = 2.0f;
    const float delta_y = -100.0f;
    const EaseOut<Back> back_out_5{Back{5.0f}};

    for (auto e: ecs::rview<bubble_text_t>()) {
        auto& state = ecs::get<bubble_text_t>(e);

        if (state.delay > 0.0f) {
            state.delay -= dt;
            continue;
        }

        state.time += dt;

        if (state.time >= time_max) {
            destroyNode(e);
            continue;
        }

        float r = math::clamp(state.time / time_max);
        float sc = 1.0f;
        float sct;
        auto off = state.offset;
        if (r < 0.5f) {
            sct = r * 2.0f;
            sc = back_out_5.calculate(easing::P3_OUT.calculate(sct));
            off = state.offset * easing::P3_OUT.calculate(sct);
        }

        setScale(e, {sc, sc});
        float2 fly_pos{0.0f, delta_y * easing::P3_OUT.calculate(r)};
        setPosition(e, state.start + off + fly_pos);
        setAlpha(e, 1.0f - (r * r * r));
    }
}

ecs::entity create_bubble_text(const float2& pos, const std::string& text, float delay) {
    auto e = create_node_2d("bb");
    auto& c = ecs::assign<bubble_text_t>(e);
    c.delay = delay;
    c.start = pos;
    float spread = 10.0f;
    c.offset.x = rand_fx.random(-spread, spread);
    TextFormat format{"Comfortaa-Regular", 32.0f};
    format.setAlignment(Alignment::Center);
    format.setTextColor(0xFFFFFFFF_argb);
    format.addShadow(0x88000000_argb, 4, {0.0f, 1.5f});

    e.assign<Display2D>(new Text2D(text, format));
    setTouchable(e, false);
    setScale(e, float2::zero);
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