#include "gui.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/scene_system.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/3d/transform_3d.hpp>
#include <ek/scenex/3d/camera_3d.hpp>
#include <ek/scenex/3d/light_3d.hpp>
#include <ek/scenex/components/movie.hpp>
#include <ek/scenex/components/node.hpp>

namespace ek {

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
inline void guiDisplayComponent(ecs::entity entity, const char* name, Func fn) {
    if (ecs::has<display_2d>(entity)) {
        auto& d = ecs::get<display_2d>(entity);
        if (d.is<C>()) {
            auto data = d.get<C>();
            if (data) {
                guiComponentPanel(name, *data, fn);
            }
        }
    }
}

void gui_movie_clip(movie_t& mc) {
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

void gui_transform_2d(transform_2d& transform) {
    float2 pos = transform.matrix.position();
    if (ImGui::DragFloat2("Position", pos.data(), 1.0f, 0.0f, 0.0f, "%.1f")) {
        transform.matrix.position(pos);
    }
    ImGui::DragFloat2("Scale", transform.scale.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::DragFloat2("Skew", transform.skew.data(), 0.1f, 0.0f, 0.0f, "%.2f");

    auto color = static_cast<float4>(transform.color_multiplier);
    if (ImGui::ColorEdit4("Color Multiplier", color.data())) {
        transform.color_multiplier = argb32_t{color};
    }

    color = static_cast<float4>(transform.color_offset);
    if (ImGui::ColorEdit4("Color Offset", color.data())) {
        transform.color_offset = argb32_t{color};
    }
}

void gui_transform_3d(transform_3d& transform) {
    ImGui::DragFloat3("Position", transform.position.data(), 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat3("Scale", transform.scale.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    float3 euler_angles = transform.rotation * 180.0f / ek::math::pi;
    if (ImGui::DragFloat3("Rotation", euler_angles.data(), 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.rotation = euler_angles * ek::math::pi / 180.0f;
    }
}

void gui_camera_3d(camera_3d& camera) {
    ImGui::DragFloatRange2("Clip Plane", &camera.near, &camera.far, 1.0f, 0.0f, 0.0f, "%.1f");
    float fov_degree = ek::math::to_degrees(camera.fov);
    if (ImGui::DragFloat("FOV", &fov_degree, 1.0f, 0.0f, 0.0f, "%.1f")) {
        camera.fov = ek::math::to_radians(fov_degree);
    }

    ImGui::Checkbox("Orthogonal", &camera.orthogonal);
    ImGui::DragFloat("Ortho Size", &camera.orthogonal_size, 1.0f, 0.0f, 0.0f, "%.1f");

    ImGui::Checkbox("Clear Color Enabled", &camera.clear_color_enabled);
    ImGui::ColorEdit4("Clear Color", camera.clear_color.data());
    ImGui::Checkbox("Clear Depth Enabled", &camera.clear_depth_enabled);
    ImGui::DragFloat("Clear Depth", &camera.clear_depth, 1.0f, 0.0f, 0.0f, "%.1f");
}

void gui_light_3d(light_3d& light) {
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

void gui_scissors_2d(scissors_2d& scissors) {
    ImGui::RectEdit("Bounds", scissors.rect.data());
}

void gui_hit_area_2d(hit_area_2d& hit_area) {
    ImGui::RectEdit("Bounds", hit_area.rect.data());
}

void gui_interactive(interactive_t& inter) {
    ImGui::Checkbox("pushed", &inter.pushed);
    ImGui::Checkbox("over", &inter.over);
    ImGui::Checkbox("bubble", &inter.bubble);
    ImGui::LabelText("cursor", inter.cursor == interactive_t::mouse_cursor::button ? "button" : "?");
}

void editDisplaySprite(drawable_sprite& sprite) {
    ImGui::LabelText("sprite", "%s", sprite.src.c_str());
    ImGui::Checkbox("hit pixels", &sprite.hit_pixels);
    ImGui::Checkbox("scale grid", &sprite.scale_grid_mode);
}

void guiTextFormat(TextFormat& format) {
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

void editDisplayText(drawable_text& tf) {
    ImGui::InputTextMultiline("Text", &tf.text);
    ImGui::RectEdit("Bounds", tf.rect.data());
    ImGui::Color32Edit("Border Color", tf.borderColor);
    ImGui::Color32Edit("Fill Color", tf.fillColor);
    ImGui::Checkbox("Hit Full Bounds", &tf.hitFullBounds);
    ImGui::Checkbox("Show Text Bounds", &tf.showTextBounds);
    guiTextFormat(tf.format);
}

void gui_inspector(ecs::entity e) {
    ImGui::PushID(e.passport());
    if (ecs::has<node_t>(e)) {
        auto& node = ecs::get<node_t>(e);
        ImGui::InputText("Name", &node.name);
        ImGui::CheckboxFlags("Visible", &node.flags, NodeFlags_Visible);
        ImGui::CheckboxFlags("Touchable", &node.flags, NodeFlags_Touchable);
        ImGui::LabelText("Layers", "%x", node.layersMask());
    }

    guiComponentPanel<transform_2d>(e, "Transform", gui_transform_2d);
    guiComponentPanel<transform_3d>(e, "Transform 3D", gui_transform_3d);
    guiComponentPanel<camera_3d>(e, "Camera 3D", gui_camera_3d);
    guiComponentPanel<light_3d>(e, "Light 3D", gui_light_3d);
    guiComponentPanel<scissors_2d>(e, "Scissors", gui_scissors_2d);
    guiComponentPanel<hit_area_2d>(e, "Hit Area", gui_hit_area_2d);
    guiComponentPanel<interactive_t>(e, "Interactive", gui_interactive);
    guiComponentPanel<event_handler_t>(e, "Event Handler", [](auto& c) {});

    guiDisplayComponent<drawable_sprite>(e, "Sprite", editDisplaySprite);
    guiDisplayComponent<drawable_quad>(e, "Rectangle", [](auto& d) {});
    guiDisplayComponent<drawable_text>(e, "Text", editDisplayText);
    guiDisplayComponent<drawable_arc>(e, "Arc", [](auto& d) {});

    guiComponentPanel<movie_t>(e, "Movie Clip", gui_movie_clip);

    if (ecs::has<script_holder>(e)) {
        auto& scripts = ecs::get<script_holder>(e).list;
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