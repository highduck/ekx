#include "minimal.hpp"
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/Display2D.hpp>

namespace ek {

Text2D& addText(ecs::entity e, const char* text) {
    auto* tf = new Text2D();
    tf->format.font.setID("mini");
    tf->format.size = 14;
    tf->format.addShadow(0x0_rgb, 8);
    tf->text = text;
    tf->format.setAlignment(Alignment::Center);
    e.get_or_create<Display2D>().drawable.reset(tf);
    return *tf;
}

ecs::entity createButton(const char* label, const std::function<void()>& fn) {
    auto e = createNode2D(label);
    auto& tf = addText(e, label);
    tf.fillColor = 0x77000000_argb;
    tf.borderColor = 0x77FFFFFF_argb;
    tf.hitFullBounds = true;
    tf.rect.set(-20, -20, 40, 40);
    e.assign<Interactive>().cursor = app::mouse_cursor::button;
    e.assign<Button>().clicked += fn;
    return e;
}

}