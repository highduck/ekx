#include "bubble_text.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/math/Easings.hpp>
#include <ek/math/Random.hpp>
#include <ek/scenex/base/TimeLayer.hpp>

namespace ek {

void BubbleText::updateAll() {
    float dt = TimeLayer::HUD->dt;
    const float time_max = 2.0f;
    const float delta_y = -100.0f;
    const EaseOut<Back> back_out_5{Back{5.0f}};

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

        float r = Math::clamp(state.time / time_max);
        float sc = 1.0f;
        float sct;
        auto off = state.offset;
        if (r < 0.5f) {
            sct = r * 2.0f;
            sc = back_out_5.calculate(easing::P3_OUT.calculate(sct));
            off = state.offset * easing::P3_OUT.calculate(sct);
        }

        auto& transform = e.get<Transform2D>();
        transform.setScale(sc);
        Vec2f fly_pos{0.0f, delta_y * easing::P3_OUT.calculate(r)};
        transform.setPosition(state.start + off + fly_pos);
        transform.color.setAlpha(1.0f - (r * r * r));
        transform.color.setAdditive(r * r * r);
    }
}

ecs::EntityApi BubbleText::create(const char* fontName, const String& text, const Vec2f& pos, float delay) {
    auto e = createNode2D();
    auto& c = e.assign<BubbleText>();
    c.delay = delay;
    c.start = pos;
    float spread = 10.0f;
    c.offset.x = rand_fx.random(-spread, spread);
    TextFormat format{fontName, 32.0f};
    format.setAlignment(Alignment::Center);
    format.setTextColor(0xFFFFFFFF_argb);
    format.addShadow(0xFF000000_argb, 4, {0.0f, 1.5f});

    e.assign<Display2D>(new Text2D(text, format));
    setTouchable(e, false);
    setScale(e, Vec2f::zero);
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