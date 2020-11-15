#include "sample_text.hpp"

#include <string>
#include <ek/scenex/text/text_drawer.hpp>
#include <ek/scenex/text/truetype_font.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/components/layout.hpp>

namespace ek {

ecs::entity createText(const char* name, const char* font, const char* text) {
    auto e = create_node_2d(name);
    auto* tf = new drawable_text();

    tf->format.font.setID(font);
    tf->format.size = 48.0f;

    // primary
    tf->format.layers[0].color = 0xFFFF77_rgb;

    // inner stroke
    tf->format.layers[1].type = TextLayerType::Stroke1;
    tf->format.layers[1].blurRadius = 1;
    tf->format.layers[1].blurIterations = 3;
    tf->format.layers[1].strength = 5;
    tf->format.layers[1].color = 0x330000_rgb;

    // outer stroke
    tf->format.layers[2].type = TextLayerType::Stroke2;
    tf->format.layers[2].blurRadius = 2;
    tf->format.layers[2].blurIterations = 3;
    tf->format.layers[2].strength = 5;
    tf->format.layers[2].color = 0xFFFFFF_rgb;

    // shadow
    tf->format.layers[3].type = TextLayerType::Shadow;
    tf->format.layers[3].blurRadius = 8;
    tf->format.layers[3].blurIterations = 3;
    tf->format.layers[3].strength = 1;
    tf->format.layers[3].offset = {4, 4};
    tf->format.layers[3].color = 0x0_rgb;

    tf->format.layersCount = 4;

    tf->text = text;
    ecs::assign<display_2d>(e).drawable.reset(tf);

    return e;
}

ecs::entity createScreenZones() {
    rect_f resolution{0, 0, 360, 480};
    auto zones = create_node_2d("zones");
    auto e = create_node_2d("zone");
    auto* q = new drawable_quad();
    q->set_gradient_vertical(0xFFFFFFFF_argb, 0x77FFFFFF_argb);
    q->rect = resolution;
    e.assign<display_2d>(q);
    e.get<transform_2d>().color_multiplier = 0x33FF00FF_argb;
    ecs::assign<layout_t>(e).fill(true, true).doSafeInsets = true;
    append(zones, e);
    e = create_node_2d("safe_zone");
    q = new drawable_quad();
    q->set_gradient_vertical(0xFFFFFFFF_argb, 0x77FFFFFF_argb);
    q->rect = resolution;
    e.assign<display_2d>(q);
    e.get<transform_2d>().color_multiplier = 0x3300FF00_argb;
    append(zones, e);
    return zones;
}

SampleText::SampleText() :
        SampleBase() {
    title = "TEXT";

    append(container, createScreenZones());

    auto bmText = createText("bmfont", "TickingTimebombBB",
                             "88:88:88\n-=98");
    get_drawable<drawable_text>(bmText).format.setAlignment(Alignment::Center);
    get_drawable<drawable_text>(bmText).format.size = 24;
    get_drawable<drawable_text>(bmText).borderColor = 0xFF000000_argb;
    get_drawable<drawable_text>(bmText).rect.set(0, 0, 360 - 40, 100);

    set_position(bmText, {20.0f, 20.0f});
    append(container, bmText);

    auto ttfText = createText("TTF-Cousine-Regular", "Cousine-Regular",
                              u8"£ü÷\n< Приветики >\n你好\nनमस्कार\nこんにちは");
    get_drawable<drawable_text>(ttfText).format.setAlignment(Alignment::Right | Alignment::Top);
    get_drawable<drawable_text>(ttfText).format.leading = -8;
    get_drawable<drawable_text>(ttfText).format.setTextColor(0xFF00FF00_argb);
    set_position(ttfText, {360 - 20, 120.0f});
    append(container, ttfText);

    auto ttfText2 = createText("TTF-Comfortaa-Regular", "Comfortaa-Regular",
                               u8"I don't know KERN TABLE.\nНо кириллица тоже есть");
    get_drawable<drawable_text>(ttfText2).format.setTextColor(0xFFFF00FF_argb);
    get_drawable<drawable_text>(ttfText2).format.size = 24;
    set_position(ttfText2, {20.0f, 340.0f});
    append(container, ttfText2);
}

void SampleText::draw() {
    // TODO: move to dev-inspector
//    if (showGlyphCache && textDrawer.format.font) {
//        textDrawer.format.font->debugDrawAtlas(512, 100 + 512);
//    }
}

void SampleText::prepareInternalResources() {
    asset_manager_t* am = resolve<basic_application>().asset_manager_;
    auto* ttfFont = new TrueTypeFont(am->scale_factor, 48, "default_glyph_cache");
    ttfFont->loadDeviceFont("Arial Unicode MS");
    auto* font = new Font(ttfFont);
    asset_t<Font>{"native"}.reset(font);
    asset_t<Font>{"Cousine-Regular"}->setFallbackFont(font);
}

void SampleText::update(float dt) {
    SampleBase::update(dt);
}

}