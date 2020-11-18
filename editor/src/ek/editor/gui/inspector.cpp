#include "gui.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/2d/UglyFilter2D.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/2d/Camera2D.hpp>

namespace ek {

template<typename T>
void selectAsset(const char* label, Res<T>& asset) {
    if (ImGui::BeginCombo(label, asset.getID().c_str())) {
        for (auto& it : Res<T>::map()) {
            auto& key = it.first;
            if (ImGui::Selectable(key.c_str(), key == asset.getID())) {
                asset.setID(key);
            }
        }
        ImGui::EndCombo();
    }
}

void guiEntityRef(const char* label, ecs::entity entity) {
    if (entity == nullptr) {
        ImGui::TextDisabled("%s: null", label);
    } else if (ecs::valid(entity)) {
        ImGui::LabelText(label, "%s", entity.get_or_default<Node>().name.c_str());
    } else {
        ImGui::TextColored({1, 0, 0, 1}, "%s: invalid", label);
    }
}

template<typename C, typename Func>
inline void guiComponentPanel(const char* name, C& data, Func fn) {
    if (ImGui::CollapsingHeader(name)) {
        ImGui::PushID(&data);
        ImGui::Indent();
        fn(data);
        ImGui::Unindent();
        ImGui::PopID();
    }
}

template<typename C, typename Func>
inline void guiComponentPanel(ecs::entity entity, const char* name, Func fn) {
    if (ecs::has<C>(entity)) {
        auto& data = ecs::get<C>(entity);
        guiComponentPanel(name, data, fn);
    }
}

template<typename C, typename Func>
inline void guiDisplayComponent(ecs::entity e, const char* name, Func fn) {
    if (e.has<Display2D>()) {
        auto& d = e.get<Display2D>();
        if (d.is<C>()) {
            auto data = d.get<C>();
            if (data) {
#ifndef NDEBUG
                ImGui::Separator();
                ImGui::Checkbox("Debug Bounds", &d.drawBounds);
#endif
                guiComponentPanel(name, *data, fn);
            }
        }
    }
}

void guiMovieClip(MovieClip& mc) {
    ImGui::LabelText("movie_data_symbol", "%s", mc.movie_data_symbol.c_str());
    const auto* data = mc.get_movie_data();
    if (data) {
        ImGui::LabelText("Total Frames", "%u", data->frames);
        ImGui::LabelText("Default FPS", "%f", data->fps);

        ImGui::DragFloat("Time", &mc.time, 1.0f, 0.0f, data->frames);
        ImGui::DragFloat("FPS", &mc.fps, 1.0f, 0.0f, 100.0f);
        ImGui::Checkbox("Playing", &mc.playing);
    }
}

void guiTransform2D(Transform2D& transform) {
    ImGui::DragFloat2("Position", transform.position.data(), 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat2("Scale", transform.scale.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::DragFloat2("Skew", transform.skew.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::DragFloat2("Origin", transform.origin.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::DragFloat2("Pivot", transform.pivot.data(), 0.1f, 0.0f, 0.0f, "%.2f");

    auto color = static_cast<float4>(transform.color.scale);
    if (ImGui::ColorEdit4("Color Scale", color.data())) {
        transform.color.scale = argb32_t{color};
    }

    color = static_cast<float4>(transform.color.offset);
    if (ImGui::ColorEdit4("Color Offset", color.data())) {
        transform.color.offset = argb32_t{color};
    }
}

void guiCamera2D(Camera2D& camera) {
    ImGui::Checkbox("Enabled", &camera.enabled);
    ImGui::Checkbox("interactive", &camera.interactive);
    ImGui::Checkbox("occlusionEnabled", &camera.occlusionEnabled);
    ImGui::DragInt("Order", &camera.order);
    ImGui::LabelText("Layers", "%x", camera.layerMask);
    guiEntityRef("Root Entity", camera.root);
    ImGui::DragFloat("Content Scale", &camera.contentScale);
    ImGui::Checkbox("Clear Color", &camera.clearColorEnabled);
    ImGui::ColorEdit4("Clear Color", camera.clearColor.data());
    ImGui::ColorEdit4("Clear Color+", camera.clearColor2.data());
    ImGui::EditRect("viewport", camera.viewport.data());
    ImGui::DragFloat2("relativeOrigin", camera.relativeOrigin.data());

    ImGui::Separator();
    ImGui::Checkbox("Draw Occlusion", &camera.debugOcclusion);
    ImGui::Checkbox("Draw Bounds", &camera.debugVisibleBounds);
    ImGui::Checkbox("Draw Hit Target", &camera.debugGizmoHitTarget);
    ImGui::Checkbox("Draw Pointer", &camera.debugGizmoPointer);
    ImGui::Checkbox("Draw Camera Gizmo", &camera.debugGizmoSelf);
    ImGui::Checkbox("Draw Script Gizmo", &camera.debugDrawScriptGizmo);
    ImGui::DragFloat("Debug Scale", &camera.debugDrawScale);
}

void guiTransform3D(Transform3D& transform) {
    ImGui::DragFloat3("Position", transform.position.data(), 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat3("Scale", transform.scale.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    float3 euler_angles = transform.rotation * 180.0f / ek::math::pi;
    if (ImGui::DragFloat3("Rotation", euler_angles.data(), 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.rotation = euler_angles * ek::math::pi / 180.0f;
    }
}

void guiCamera3D(Camera3D& camera) {
    ImGui::DragFloatRange2("Clip Plane", &camera.near, &camera.far, 1.0f, 0.0f, 0.0f, "%.1f");
    float fov_degree = ek::math::to_degrees(camera.fov);
    if (ImGui::DragFloat("FOV", &fov_degree, 1.0f, 0.0f, 0.0f, "%.1f")) {
        camera.fov = ek::math::to_radians(fov_degree);
    }

    ImGui::Checkbox("Orthogonal", &camera.orthogonal);
    ImGui::DragFloat("Ortho Size", &camera.orthogonalSize, 1.0f, 0.0f, 0.0f, "%.1f");

    ImGui::Checkbox("Clear Color Enabled", &camera.clearColorEnabled);
    ImGui::ColorEdit4("Clear Color", camera.clearColor.data());
    ImGui::Checkbox("Clear Depth Enabled", &camera.clearDepthEnabled);
    ImGui::DragFloat("Clear Depth", &camera.clearDepth, 1.0f, 0.0f, 0.0f, "%.1f");
}

void guiLight3D(Light3D& light) {
    if (light.type == light_3d_type::directional) {
        ImGui::Text("Directional Light");
    } else if (light.type == light_3d_type::point) {
        ImGui::Text("Point Light");
    } else if (light.type == light_3d_type::spot) {
        ImGui::Text("Spot Light");
    }
    ImGui::ColorEdit3("Ambient", light.ambient.data());
    ImGui::ColorEdit3("Diffuse", light.diffuse.data());
    ImGui::ColorEdit3("Specular", light.specular.data());

    ImGui::DragFloat("Radius", &light.radius, 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat("Falloff", &light.falloff, 0.1f, 0.0f, 0.0f, "%.1f");
}

void guiScissors(Scissors& scissors) {
    ImGui::EditRect("Bounds", scissors.rect.data());
}

void guiHitArea(HitArea& hit_area) {
    ImGui::EditRect("Bounds", hit_area.rect.data());
}

void guiInteractive(interactive_t& inter) {
    ImGui::Checkbox("pushed", &inter.pushed);
    ImGui::Checkbox("over", &inter.over);
    ImGui::Checkbox("bubble", &inter.bubble);
    ImGui::LabelText("cursor", inter.cursor == interactive_t::mouse_cursor::button ? "button" : "?");
}

void editDisplaySprite(Sprite2D& sprite) {
    selectAsset<Sprite>("Sprite", sprite.src);
    ImGui::Checkbox("Scale Grid", &sprite.scale_grid_mode);
    ImGui::Checkbox("Hit Pixels", &sprite.hit_pixels);
}

void editDisplayRectangle(Quad2D& quad) {
    ImGui::EditRect("Bounds", quad.rect.data());
    ImGui::Color32Edit("Color LT", quad.colors[0]);
    ImGui::Color32Edit("Color RT", quad.colors[1]);
    ImGui::Color32Edit("Color RB", quad.colors[2]);
    ImGui::Color32Edit("Color LB", quad.colors[3]);
}

void editDisplayArc(Arc2D& arc) {
    selectAsset<Sprite>("Sprite", arc.sprite);
    ImGui::DragFloat("Angle", &arc.angle);
    ImGui::DragFloat("Radius", &arc.radius);
    ImGui::DragFloat("Line Width", &arc.line_width);
    ImGui::DragInt("Segments", &arc.segments);
    ImGui::Color32Edit("Color Inner", arc.color_inner);
    ImGui::Color32Edit("Color Outer", arc.color_outer);
}

void editParticleRenderer2D(ParticleRenderer2D& p) {
    guiEntityRef("Target", p.target);
    ImGui::Checkbox("Cycled Mode", &p.cycled);
}

void guiTextFormat(TextFormat& format) {
    selectAsset<Font>("Font", format.font);
    ImGui::DragFloat("Size", &format.size, 1, 8, 128, "%f");
    ImGui::DragFloat("Leading", &format.leading, 1, 0, 128, "%f");
    ImGui::DragFloat("Spacing", &format.letterSpacing, 1, -128, 128, "%f");
    ImGui::DragFloat2("Alignment", format.alignment.data(), 0.5f, 0.0f, 1.0f);
    ImGui::Checkbox("Kerning", &format.kerning);
    ImGui::Checkbox("Underline", &format.underline);
    ImGui::Checkbox("Strikethrough", &format.strikethrough);

    ImGui::SliderInt("Layers", &format.layersCount, 0, TextFormat::LayersMax);
    if (ImGui::Checkbox("Show all glyph bounds", &format.layers[0].showGlyphBounds)) {
        bool sgb = format.layers[0].showGlyphBounds;
        for (int i = 0; i < TextFormat::LayersMax; ++i) {
            format.layers[i].showGlyphBounds = sgb;
        }
    }

    ImGui::Indent();
    for (int i = 0; i < format.layersCount; ++i) {
        auto& layer = format.layers[i];
        guiTextLayerEffect(layer);
    }
    ImGui::Unindent();
}

void editDisplayText(Text2D& tf) {
    ImGui::InputTextMultiline("Text", &tf.text);
    ImGui::EditRect("Bounds", tf.rect.data());
    ImGui::Color32Edit("Border Color", tf.borderColor);
    ImGui::Color32Edit("Fill Color", tf.fillColor);
    ImGui::Checkbox("Hit Full Bounds", &tf.hitFullBounds);
    ImGui::Checkbox("Show Text Bounds", &tf.showTextBounds);
    guiTextFormat(tf.format);
}

void guiLayout(LayoutRect& layout) {
    ImGui::Checkbox("Fill X", &layout.fill_x);
    ImGui::Checkbox("Fill Y", &layout.fill_y);
    ImGui::Checkbox("Align X", &layout.align_x);
    ImGui::Checkbox("Align Y", &layout.align_y);
    ImGui::Checkbox("Use Safe Insets", &layout.doSafeInsets);
    ImGui::EditRect("Extra Fill", layout.fill_extra.data());
    ImGui::DragFloat2("Align X (rel, abs)", layout.x.data());
    ImGui::DragFloat2("Align Y (rel, abs)", layout.y.data());

    ImGui::EditRect("Rect", layout.rect.data());
    ImGui::EditRect("Safe Rect", layout.safeRect.data());
}

void guiUglyFilter2D(UglyFilter2D& filters) {
    ImGui::Checkbox("Enabled", &filters.enabled);
    for (int i = 0; i < filters.filters.size(); ++i) {
        auto& filter = filters.filters[i];
        ImGui::PushID(filters.filters.data() + i);
        ImGui::LabelText("Type", "%d", filter.type);
        ImGui::Color32Edit("Color", filter.color);
        ImGui::DragFloat2("Offset", filter.offset.data());
        ImGui::DragFloat2("Blur", filter.blur.data());
        ImGui::DragInt("Quality", (int*) &filter.quality);
        ImGui::PopID();
    }
}

void guiParticleEmitter2D(ParticleEmitter2D& emitter) {
    ImGui::Checkbox("Enabled", &emitter.enabled);
    ImGui::Text("_Time: %f", emitter.time);
    guiEntityRef("Layer", emitter.layer);
    ImGui::LabelText("Particle ID", "%s", emitter.particle.c_str());
    ImGui::DragFloat2("Offset", emitter.position.data());
    ImGui::Separator();

    // data
    ImGui::EditRect("Rect", emitter.data.rect.data());
    ImGui::DragFloat2("Offset", emitter.data.offset.data());
    ImGui::DragInt("Burst", &emitter.data.burst);
    ImGui::DragFloat("Interval", &emitter.data.interval);
    ImGui::Separator();

    ImGui::DragFloat2("burst_rotation_delta", emitter.data.burst_rotation_delta.data());
    ImGui::DragFloat2("speed", emitter.data.speed.data());
    ImGui::DragFloat2("acc", emitter.data.acc.data());
    ImGui::DragFloat2("dir", emitter.data.dir.data());
}

void guiParticleLayer2D(ParticleLayer2D& layer) {
    ImGui::Checkbox("Keep Alive", &layer.keep_alive);
    ImGui::LabelText("Num Particles", "%lu", layer.particles.size());
}

void gui_inspector(ecs::entity e) {
    ImGui::PushID(e.passport());
    if (ecs::has<Node>(e)) {
        auto& node = ecs::get<Node>(e);
        ImGui::InputText("Name", &node.name);
        ImGui::CheckboxFlags("Visible", &node.flags, Node::Visible);
        ImGui::CheckboxFlags("Touchable", &node.flags, Node::Touchable);
        ImGui::LabelText("Layers", "%x", node.layersMask());
    }

    guiComponentPanel<UglyFilter2D>(e, "UglyFilter2D", guiUglyFilter2D);
    guiComponentPanel<Transform2D>(e, "Transform2D", guiTransform2D);
    guiComponentPanel<Camera2D>(e, "Camera2D", guiCamera2D);

    guiComponentPanel<Transform3D>(e, "Transform 3D", guiTransform3D);
    guiComponentPanel<Camera3D>(e, "Camera 3D", guiCamera3D);
    guiComponentPanel<Light3D>(e, "Light 3D", guiLight3D);
    guiComponentPanel<Scissors>(e, "Scissors", guiScissors);
    guiComponentPanel<HitArea>(e, "Hit Area", guiHitArea);
    guiComponentPanel<interactive_t>(e, "Interactive", guiInteractive);
    guiComponentPanel<event_handler_t>(e, "Event Handler", [](auto& c) {});
    guiComponentPanel<LayoutRect>(e, "Layout", guiLayout);

    // particles
    guiComponentPanel<ParticleEmitter2D>(e, "ParticleEmitter2D", guiParticleEmitter2D);
    guiComponentPanel<ParticleLayer2D>(e, "ParticleLayer2D", guiParticleLayer2D);

    // display2d
    guiDisplayComponent<Sprite2D>(e, "Sprite", editDisplaySprite);
    guiDisplayComponent<Quad2D>(e, "Rectangle", editDisplayRectangle);
    guiDisplayComponent<Text2D>(e, "Text", editDisplayText);
    guiDisplayComponent<Arc2D>(e, "Arc", editDisplayArc);
    guiDisplayComponent<ParticleRenderer2D>(e, "ParticleRenderer2D", editParticleRenderer2D);

    guiComponentPanel<MovieClip>(e, "Movie Clip", guiMovieClip);

    if (e.has<script_holder>()) {
        auto& scripts = e.get<script_holder>().list;
        for (auto& script : scripts) {
            script->gui_inspector();
        }
    }

    ImGui::PopID();
}

void guiInspectorWindow(bool* p_open) {
    if (ImGui::Begin("Inspector", p_open)) {
        if (!hierarchySelectionList.empty()) {
            if (hierarchySelectionList.size() > 1) {
                ImGui::LabelText("Multiple Selection", "%lu", hierarchySelectionList.size());
            } else {
                gui_inspector(hierarchySelectionList[0]);
            }
        }
    }
    ImGui::End();
}


}