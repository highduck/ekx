#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/components/button.hpp>
#include "minimal.hpp"

namespace ek {

drawable_text& addText(ecs::entity e, const char* text) {
    auto* tf = new drawable_text();
    tf->format.font.setID("mini");
    tf->format.size = 16;
    tf->format.addShadow(0x0_rgb, 8);
    tf->text = text;
    tf->format.setAlignment(Alignment::Center);
    ecs::get_or_create<display_2d>(e).drawable.reset(tf);
    return *tf;
}

ecs::entity createButton(const char* label, const std::function<void()>& fn) {
    auto e = create_node_2d(label);
    auto& tf = addText(e, label);
    tf.fillColor = 0x77000000_argb;
    tf.borderColor = 0x77FFFFFF_argb;
    tf.hitFullBounds = true;
    tf.rect.set(-20, -20, 40, 40);
    ecs::assign<interactive_t>(e).cursor = app::mouse_cursor::button;
    ecs::assign<button_t>(e).clicked += fn;
    return e;
}

}