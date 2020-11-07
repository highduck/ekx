#include "sample_text.hpp"

#include <string>
#include <ek/scenex/text/text_drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/text/truetype_font.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
//#include <imgui.h>

namespace ek {

SampleText::SampleText() :
        SampleBase() {
    textDrawer.font = asset_t<Font>{"Cousine-Regular"};
    textDrawer.nativeFont = asset_t<Font>{"native"};
    textDrawer.fontSize = 48.0f;// * app::g_app.content_scale;

    outline1.radius = 1;
    outline1.iterations = 1;
    outline1.strength = 7;
    outline1.color = {0.2, 0, 0, 1};

    outline2.radius = 2;
    outline2.iterations = 2;
    outline2.strength = 7;

    shadow.radius = 4;
    shadow.iterations = 3;
    shadow.strength = 1;
    shadow.color = {0, 0, 0, 1};
}

void SampleText::draw() {

    auto& ap = resolve<basic_application>();
    auto sf = ap.asset_manager_->scale_factor;
    draw2d::state.save_matrix();
    draw2d::state.scale(sf, sf);
    std::string text{u8"£ü÷\n< Приветики >\n你好\nनमस्कार\nこんにちは"};

    textDrawer.font = asset_t<Font>("Cousine-Regular");
    textDrawer.position = {20.0f, 220.0f};
    textDrawer.setBlur(shadow.radius, shadow.iterations, shadow.strength);
    textDrawer.textColor = argb32_t{shadow.color};
    textDrawer.draw(text);

    textDrawer.position -= {4.0f, 4.0f};

    textDrawer.setBlur(outline2.radius, outline2.iterations, outline2.strength);
    textDrawer.textColor = argb32_t{outline2.color};
    textDrawer.draw(text);

    textDrawer.setBlur(outline1.radius, outline1.iterations, outline1.strength);
    textDrawer.textColor = argb32_t{outline1.color};
    textDrawer.draw(text);

    textDrawer.textColor = 0xFFFF77_rgb;
    textDrawer.setBlur(0, 0, 0);
    textDrawer.draw(text);
//        font->draw(text, 16, pos + float2::one, 0x0_rgb, 16);
//        font->draw(text, 16, pos, 0xFFFFFF_rgb, 16);

    textDrawer.font = asset_t<Font>("TickingTimebombBB");
    textDrawer.position = {20.0f, 100.0f};
    textDrawer.setBlur(0, 0, 0);
    textDrawer.textColor = 0xFFFFFFFF_argb;
    textDrawer.draw("88:88:88\n-=98");

    draw2d::state.restore_matrix();

    if (drawGlyphCache) {
        if (textDrawer.nativeFont) {
            textDrawer.nativeFont->debugDrawAtlas(512, 100);
        }

        if (textDrawer.font) {
            textDrawer.font->debugDrawAtlas(512, 100 + 512);
        }
    }
}

void SampleText::prepareInternalResources() {
    asset_manager_t* am = resolve<basic_application>().asset_manager_;
    auto* ttfFont = new TrueTypeFont(am->scale_factor, 48, 4096);
    ttfFont->loadDeviceFont("Arial Unicode MS");
    asset_t<Font>{"native"}.reset(new Font(ttfFont));
}


void SampleText::update(float dt) {
    SampleBase::update(dt);

//    ImGui::Begin("Text");
//
//    ImGui::DragFloat("Font Size", &textDrawer.fontSize);
//    ImGui::Checkbox("Show Glyph Cache", &drawGlyphCache);
//
//    ImGui::Text("Outline 1");
//    outline1.gui();
//
//    ImGui::Text("Outline 2");
//    outline2.gui();
//
//    ImGui::Text("Shadow");
//    shadow.gui();
//
//    ImGui::End();
}

void SampleBlurProperties::gui() {
//    ImGui::PushID(this);
//    ImGui::DragFloat("Half Radius", &radius, 1, 0, 8);
//    ImGui::DragInt("Iterations", &iterations, 1, 0, 3);
//    ImGui::DragInt("Strength", &strength, 1, 0, 7);
//    ImGui::ColorEdit4("Color", color.data());
//    ImGui::PopID();
}

}