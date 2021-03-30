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
#include <ek/scenex/3d/StaticMesh.hpp>
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

void guiEntityRef(const char* label, ecs::EntityRef ref) {
    if (ref == nullptr) {
        ImGui::TextDisabled("%s: null", label);
    } else if (ref.valid()) {
        ImGui::LabelText(label, "%s", ref.get().get_or_default<NodeName>().name.c_str());
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
inline void guiComponentPanel(ecs::EntityApi entity, const char* name, Func fn) {
    if (entity.has<C>()) {
        auto& data = entity.get<C>();
        guiComponentPanel(name, data, fn);
    }
}

template<typename C, typename Func>
inline void guiDisplayComponent(ecs::EntityApi e, const char* name, Func fn) {
    if (e.has<Display2D>()) {
        auto& d = e.get<Display2D>();
        if (d.is<C>()) {
            auto data = d.tryGet<C>();
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
    auto pos = transform.getPosition();
    auto scale = transform.getScale();
    auto skew = transform.getSkew();
    if(ImGui::DragFloat2("Position", pos.data(), 1.0f, 0.0f, 0.0f, "%.1f")) {
        transform.setPosition(pos);
    }
    if(ImGui::DragFloat2("Scale", scale.data(), 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.setScale(scale);
    }
    if(ImGui::DragFloat2("Skew", skew.data(), 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.setSkew(skew);
    }
//    ImGui::DragFloat2("Origin", transform.origin.data(), 0.1f, 0.0f, 0.0f, "%.2f");
//    ImGui::DragFloat2("Pivot", transform.pivot.data(), 0.1f, 0.0f, 0.0f, "%.2f");

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
    ImGui::DragFloatRange2("Clip Plane", &camera.zNear, &camera.zFar, 1.0f, 0.0f, 0.0f, "%.1f");
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

void guiMeshRenderer(MeshRenderer& renderer) {
    ImGui::Checkbox("Cast Shadows", &renderer.castShadows);
    ImGui::Checkbox("Receive Shadows", &renderer.receiveShadows);
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

void guiBounds2D(Bounds2D& bounds) {
    ImGui::EditRect("Rect", bounds.rect.data());
    ImGui::Checkbox("Hit Area", &bounds.hitArea);
    ImGui::Checkbox("Scissors", &bounds.scissors);
    ImGui::Checkbox("Culling (WIP)", &bounds.culling);
}

void guiInteractive(Interactive& inter) {
    ImGui::Checkbox("pushed", &inter.pushed);
    ImGui::Checkbox("over", &inter.over);
    ImGui::Checkbox("bubble", &inter.bubble);
    ImGui::LabelText("cursor", inter.cursor == Interactive::mouse_cursor::button ? "button" : "?");
}

void editDisplaySprite(Sprite2D& sprite) {
    selectAsset<Sprite>("Sprite", sprite.src);
    ImGui::Checkbox("Hit Pixels", &sprite.hit_pixels);
}

void editDisplayNinePatch(NinePatch2D& ninePatch) {
    selectAsset<Sprite>("Sprite", ninePatch.src);
    // TODO: scale, size
    //ImGui::Checkbox("Scale Grid", &ninePatch.scale_grid_mode);
    ImGui::Checkbox("Hit Pixels", &ninePatch.hit_pixels);
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

void gui_inspector(ecs::EntityApi e) {
    ImGui::PushID(e.index);
    ImGui::LabelText("Passport", "ID: %d, Version: %d", e.index, ecs::the_world.generation(e.index));
    if (e.has<NodeName>()) {
        ImGui::InputText("Name", &e.get<NodeName>().name);
    }
    if (e.has<Node>()) {
        auto& node = e.get<Node>();
        int flags = node.flags;
        ImGui::CheckboxFlags("Visible", &flags, Node::Visible);
        ImGui::CheckboxFlags("Touchable", &flags, Node::Touchable);
        ImGui::LabelText("Layers", "%x", node.layersMask());
        node.flags = flags;
    }

    guiComponentPanel<UglyFilter2D>(e, "UglyFilter2D", guiUglyFilter2D);
    guiComponentPanel<Transform2D>(e, "Transform2D", guiTransform2D);
    guiComponentPanel<Camera2D>(e, "Camera2D", guiCamera2D);
    guiComponentPanel<Bounds2D>(e, "Bounds2D", guiBounds2D);
    guiComponentPanel<LayoutRect>(e, "Layout", guiLayout);

    guiComponentPanel<Transform3D>(e, "Transform 3D", guiTransform3D);
    guiComponentPanel<Camera3D>(e, "Camera 3D", guiCamera3D);
    guiComponentPanel<Light3D>(e, "Light 3D", guiLight3D);
    guiComponentPanel<MeshRenderer>(e, "Mesh Renderer", guiMeshRenderer);

    guiComponentPanel<Interactive>(e, "Interactive", guiInteractive);
    guiComponentPanel<NodeEventHandler>(e, "Event Handler", [](auto& c) {});

    // particles
    guiComponentPanel<ParticleEmitter2D>(e, "ParticleEmitter2D", guiParticleEmitter2D);
    guiComponentPanel<ParticleLayer2D>(e, "ParticleLayer2D", guiParticleLayer2D);

    // display2d
    guiDisplayComponent<Sprite2D>(e, "Sprite2D", editDisplaySprite);
    guiDisplayComponent<NinePatch2D>(e, "NinePatch2D", editDisplayNinePatch);
    guiDisplayComponent<Quad2D>(e, "Quad2D", editDisplayRectangle);
    guiDisplayComponent<Text2D>(e, "Text2D", editDisplayText);
    guiDisplayComponent<Arc2D>(e, "Arc2D", editDisplayArc);
    guiDisplayComponent<ParticleRenderer2D>(e, "ParticleRenderer2D", editParticleRenderer2D);

    guiComponentPanel<MovieClip>(e, "Movie Clip", guiMovieClip);

    if (e.has<ScriptHolder>()) {
        auto& scripts = e.get<ScriptHolder>().list;
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