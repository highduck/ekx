#include "sample_text.hpp"

#include <string>
#include <ek/scenex/text/text_drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/text/truetype_font.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <imgui.h>

namespace ek {

SampleText::SampleText() :
        SampleBase() {
    textDrawer.fallback = asset_t<Font>{"native"};

    textDrawer.format.font = asset_t<Font>{"Cousine-Regular"};
    textDrawer.format.size = 48.0f;

    // primary
    textDrawer.format.layers[0].color = 0xFFFF77_rgb;

    // inner stroke
    textDrawer.format.layers[1].blurRadius = 1;
    textDrawer.format.layers[1].blurIterations = 1;
    textDrawer.format.layers[1].strength = 7;
    textDrawer.format.layers[1].color = 0x330000_rgb;

    // outer stroke
    textDrawer.format.layers[2].blurRadius = 2;
    textDrawer.format.layers[2].blurIterations = 2;
    textDrawer.format.layers[2].strength = 7;
    textDrawer.format.layers[2].color = 0xFFFFFF_rgb;

    // shadow
    textDrawer.format.layers[3].blurRadius = 8;
    textDrawer.format.layers[3].blurIterations = 3;
    textDrawer.format.layers[3].strength = 1;
    textDrawer.format.layers[3].offset = {4, 4};
    textDrawer.format.layers[3].color = 0x0_rgb;

    textDrawer.format.layersCount = 4;
}

void SampleText::draw() {

    auto& ap = resolve<basic_application>();
    auto sf = ap.asset_manager_->scale_factor;
    draw2d::state.save_matrix();
    draw2d::state.scale(sf, sf);

    textDrawer.format.font = asset_t<Font>("Cousine-Regular");
//    textDrawer.format.layers[0].color = 0xFFFF77_rgb;
    textDrawer.position = {20.0f, 220.0f};
    textDrawer.draw(u8"£ü÷\n< Приветики >\n你好\nनमस्कार\nこんにちは");

    textDrawer.format.font = asset_t<Font>("TickingTimebombBB");
//    textDrawer.format.layers[0].color = 0xFFFFFF_rgb;
    textDrawer.position = {20.0f, 100.0f};
    textDrawer.draw("88:88:88\n-=98");

    draw2d::state.restore_matrix();

    if (drawGlyphCache) {
        if (textDrawer.fallback) {
            textDrawer.fallback->debugDrawAtlas(512, 100);
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

void guiEffectLayer(TextLayerEffect& layer) {
    ImGui::PushID(&layer);
    ImGui::Checkbox("Draw Glyph Bounds", &layer.glyphBounds);
    ImGui::DragFloat("Radius", &layer.blurRadius, 1, 0, 8);
    int iterations = layer.blurIterations;
    int strength = layer.strength;
    ImGui::DragInt("Iterations", &iterations, 1, 0, 3);
    ImGui::DragInt("Strength", &strength, 1, 0, 7);
    layer.blurIterations = iterations;
    layer.strength = strength;

    ImGui::DragFloat2("Offset", layer.offset.data(), 1, 0, 8);

    float4 color{layer.color};
    if(ImGui::ColorEdit4("Color", color.data())) {
        layer.color = argb32_t{color};
    }

    ImGui::PopID();
}

void SampleText::update(float dt) {
    SampleBase::update(dt);

    ImGui::Begin("Text");

    ImGui::DragFloat("Font Size", &textDrawer.format.size);
    ImGui::Checkbox("Show Glyph Cache", &drawGlyphCache);

    ImGui::Text("Text");
    guiEffectLayer(textDrawer.format.layers[0]);

    ImGui::Text("Outline 1");
    guiEffectLayer(textDrawer.format.layers[1]);

    ImGui::Text("Outline 2");
    guiEffectLayer(textDrawer.format.layers[2]);

    ImGui::Text("Shadow");
    guiEffectLayer(textDrawer.format.layers[3]);

    ImGui::End();
}

}