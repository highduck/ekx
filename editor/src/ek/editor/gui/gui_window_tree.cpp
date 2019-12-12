#include "gui_window_tree.h"
#include "gui_window_stats.h"

#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/name.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/script.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/components/event_handler.hpp>
#include <ek/scenex/3d/transform_3d.hpp>
#include <ek/scenex/3d/camera_3d.hpp>
#include <ek/math/common.hpp>
#include <ek/scenex/3d/light_3d.hpp>

namespace ek {

void gui_entity(ecs::entity e, bool parent_visible = true);

void gui_entity_simple(ecs::entity e);

void gui_entity_node(ecs::entity e, bool parent_visible);

void gui_entity_simple(ecs::entity e) {
    ImGui::LabelText("entity", "id: %i (v.%i)", e.index(), e.version());
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        ecs::destroy(e);
    }
}

void gui_entity(ecs::entity e, bool parent_visible) {
    ImGui::PushID(e.index());
    ImGui::BeginGroup();

    if (ecs::valid(e)) {
        if (ecs::has<node_t>(e)) {
            gui_entity_node(e, parent_visible);
        } else {
            gui_entity_simple(e);
        }
    }
    ImGui::EndGroup();
    ImGui::PopID();

}

static std::vector<ecs::entity> selected_entities{};

void gui_entity_node(ecs::entity e, bool parent_visible) {
    std::string name = "Node #" + std::to_string(e.index());
    if (ecs::has<name_t>(e)) {
        name = ecs::get<name_t>(e).name;
    }
    bool has_children = ecs::get<node_t>(e).child_first != nullptr;
    bool visible = ecs::get_or_default<node_state_t>(e).visible;

    bool selected = std::find(selected_entities.begin(),
                              selected_entities.end(),
                              e) != selected_entities.end();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                               | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!has_children) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    ImGui::Columns(2);
    ImGui::PushStyleColor(ImGuiCol_Text, (parent_visible && visible) ? 0xFFFFFFFF : 0x77FFFFFF);
    bool opened = ImGui::TreeNodeEx("node", flags, "%s [%u][%u]", name.c_str(), e.index(), e.version());
    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked()) {
        selected_entities.clear();
        selected_entities.push_back(e);
    }

//    std::string info = std::to_string(e.index()) + " (" + std::to_string(e.version()) + ")";
//    ImGui::SetCursorPosX(ImGui::GetItemRectSize().x - ImGui::CalcTextSize(info.c_str()).x);
//    ImGui::TextDisabled("%u (%u)", e.index(), e.version());

    ImGui::NextColumn();
    ImGui::SetColumnOffset(1, ImGui::GetWindowContentRegionWidth() - 50);
    if (ImGui::Button(visible ? "<o>" : "<->")) {
        visible = !visible;
        ecs::get_or_create<node_state_t>(e).visible = visible;
    }
    ImGui::NextColumn();

    if (opened) {
        if (ecs::valid(e) && ecs::has<node_t>(e)) {
            auto it = ecs::get<node_t>(e).child_first;
            while (it != ecs::null && ecs::valid(it)) {
                auto child = it;
                it = ecs::get<node_t>(it).sibling_next;
                gui_entity(child, visible);
            }
        }
        ImGui::TreePop();
    }
}

void gui_window_tree() {
    ImGui::Begin("tree");

    const auto& pool = ecxx::get_world().pool();
    ImGui::LabelText("active", "%u", pool.size());
    ImGui::LabelText("free", "%u", pool.available_for_recycling());

    if (!selected_entities.empty()) {
        if (ImGui::Button(selected_entities.size() > 1 ? "delete all" : "delete")) {
            for (auto e : selected_entities) {
                if (ecs::valid(e)) {
                    if (ecs::has<node_t>(e)) {
                        erase_node_component(e);
                    }
                    ecs::destroy(e);
                }
            }
            selected_entities.clear();
        }
    } else {
        ImGui::Button("dummy");
    }

    ecs::each([](auto e) {
        if (!ecs::has<node_t>(e)
            || ecs::get<node_t>(e).parent == ecs::null) {
            gui_entity(e);
        }
    });

    ImGui::End();

//    ImGui::ShowDemoWindow();
//    ImGui::ShowAboutWindow();
//    ImGui::ShowMetricsWindow();
//    ImGui::ShowUserGuide();
}

void gui_transform_2d(ecs::entity entity) {
    auto& transform = ecs::get<transform_2d>(entity);
    if (ImGui::CollapsingHeader("Transform 2D")) {
        float2 pos = transform.matrix.position();
        if (ImGui::DragFloat2("Position", pos.data_, 1.0f, 0.0f, 0.0f, "%.1f", 1.0f)) {
            transform.matrix.position(pos);
        }
        ImGui::DragFloat2("Scale", transform.scale.data_, 0.1f, 0.0f, 0.0f, "%.2f", 0.1f);
        ImGui::DragFloat2("Skew", transform.skew.data_, 0.1f, 0.0f, 0.0f, "%.2f", 0.1f);

        auto color = static_cast<float4>(transform.colorMultiplier);
        if (ImGui::ColorEdit4("Color Multiplier", color.data_)) {
            transform.colorMultiplier = argb32_t{color};
        }

        color = static_cast<float4>(transform.colorOffset);
        if (ImGui::ColorEdit4("Color Offset", color.data_)) {
            transform.colorOffset = argb32_t{color};
        }
    }

}

void gui_transform_3d(ecs::entity entity) {
    auto& transform = ecs::get<transform_3d>(entity);
    if (ImGui::CollapsingHeader("Transform 3D")) {
        ImGui::DragFloat3("Position", transform.position.data_, 1.0f, 0.0f, 0.0f, "%.1f", 1.0f);
        ImGui::DragFloat3("Scale", transform.scale.data_, 0.1f, 0.0f, 0.0f, "%.2f", 0.1f);
        float3 euler_angles = transform.rotation * 180.0f / ek::math::pi;
        if (ImGui::DragFloat3("Rotation", euler_angles.data_, 0.1f, 0.0f, 0.0f, "%.2f", 0.1f)) {
            transform.rotation = euler_angles * ek::math::pi / 180.0f;
        }
    }
}

void gui_camera_3d(ecs::entity entity) {
    auto& camera = ecs::get<camera_3d>(entity);
    if (ImGui::CollapsingHeader("Camera 3D")) {
        ImGui::DragFloatRange2("Clip Plane", &camera.near, &camera.far, 1.0f, 0.0f, 0.0f, "%.1f");
        float fov_degree = ek::math::to_degrees(camera.fov);
        if (ImGui::DragFloat("FOV", &fov_degree, 1.0f, 0.0f, 0.0f, "%.1f")) {
            camera.fov = ek::math::to_radians(fov_degree);
        }

        ImGui::Checkbox("Orthogonal", &camera.orthogonal);
        ImGui::DragFloat("Ortho Size", &camera.orthogonal_size, 1.0f, 0.0f, 0.0f, "%.1f");

        ImGui::Checkbox("Clear Color Enabled", &camera.clear_color_enabled);
        ImGui::ColorEdit4("Clear Color", camera.clear_color.data_);
        ImGui::Checkbox("Clear Depth Enabled", &camera.clear_depth_enabled);
        ImGui::DragFloat("Clear Depth", &camera.clear_depth, 1.0f, 0.0f, 0.0f, "%.1f");
    }
}

void gui_light_3d(ecs::entity entity) {
    auto& light = ecs::get<light_3d>(entity);
    if (ImGui::CollapsingHeader("Light 3D")) {
        if (light.type == light_3d_type::directional) {
            ImGui::Text("Directional Light");
        } else if (light.type == light_3d_type::point) {
            ImGui::Text("Point Light");
        } else if (light.type == light_3d_type::spot) {
            ImGui::Text("Spot Light");
        }
        ImGui::ColorEdit3("Ambient", light.ambient.data_);
        ImGui::ColorEdit3("Diffuse", light.diffuse.data_);
        ImGui::ColorEdit3("Specular", light.specular.data_);

        ImGui::DragFloat("Radius", &light.radius, 1.0f, 0.0f, 0.0f, "%.1f");
        ImGui::DragFloat("Falloff", &light.falloff, 0.1f, 0.0f, 0.0f, "%.1f");
    }
}

void imgui_rect(rect_f& rc) {
    ImGui::DragFloat2("Position", rc.position.data_);
    ImGui::DragFloat2("Size", rc.size.data_);
}

void gui_scissors_2d(ecs::entity entity) {
    auto& scissors = ecs::get<scissors_2d>(entity);
    if (ImGui::CollapsingHeader("Scissors 2D")) {
        imgui_rect(scissors.rect);
    }
}

void gui_hit_area_2d(ecs::entity entity) {
    auto& hit_area = ecs::get<hit_area_2d>(entity);
    if (ImGui::CollapsingHeader("Hit Area 2D")) {
        imgui_rect(hit_area.rect);
    }
}

void gui_interactive(ecs::entity entity) {
    auto& inter = ecs::get<interactive_t>(entity);
    if (ImGui::CollapsingHeader("Interactive")) {
        ImGui::Checkbox("pushed", &inter.pushed);
        ImGui::Checkbox("over", &inter.over);
        ImGui::Checkbox("bubble", &inter.bubble);
        ImGui::LabelText("cursor", inter.cursor == interactive_t::mouse_cursor::button ? "button" : "?");
    }
}

void gui_event_handler(ecs::entity entity) {
    auto& comp = ecs::get<event_handler_t>(entity);
    if (ImGui::CollapsingHeader("Event Handler")) {
    }
}

void gui_display_2d(ecs::entity entity) {
    auto& comp = ecs::get<display_2d>(entity);
    if (comp.drawable) {
        auto* drawable = comp.drawable.get();
        auto drawable_type = drawable->get_type_id();
        if (drawable_type == drawable_sprite::type_id) {
            auto* sprite = static_cast<drawable_sprite*>(drawable);
            if (ImGui::CollapsingHeader("Display 2D - Sprite")) {
                ImGui::LabelText("sprite", "%s", sprite->src.c_str());
                ImGui::Checkbox("hit pixels", &sprite->hit_pixels);
                ImGui::Checkbox("scale grid", &sprite->scale_grid_mode);
            }
        } else if (drawable_type == drawable_quad::type_id) {
            if (ImGui::CollapsingHeader("Display 2D - Quad")) {}
        } else if (drawable_type == drawable_text::type_id) {
            if (ImGui::CollapsingHeader("Display 2D - Text")) {}
        } else if (drawable_type == drawable_arc::type_id) {
            if (ImGui::CollapsingHeader("Display 2D - Arc")) {}
        }
    } else {
        if (ImGui::CollapsingHeader("Display 2D - unknown")) {}
    }
}

void gui_inspector(ecs::entity e) {
    if (ecs::has<name_t>(e)) {
        auto& name = ecs::get<name_t>(e);
        if (ImGui::InputText("Name", &name.name)) {

        }
    }
    if (ecs::has<node_state_t>(e)) {
        auto& config = ecs::get<node_state_t>(e);
        ImGui::Checkbox("Visible", &config.visible);
        ImGui::Checkbox("Touchable", &config.touchable);
        ImGui::LabelText("Layer Mask", "%u", config.layer_mask);
    }
    if (ecs::has<transform_2d>(e)) {
        gui_transform_2d(e);
    }
    if (ecs::has<transform_3d>(e)) {
        gui_transform_3d(e);
    }
    if (ecs::has<camera_3d>(e)) {
        gui_camera_3d(e);
    }
    if (ecs::has<light_3d>(e)) {
        gui_light_3d(e);
    }
    if (ecs::has<scissors_2d>(e)) {
        gui_scissors_2d(e);
    }
    if (ecs::has<hit_area_2d>(e)) {
        gui_hit_area_2d(e);
    }
    if (ecs::has<interactive_t>(e)) {
        gui_interactive(e);
    }
    if (ecs::has<event_handler_t>(e)) {
        gui_event_handler(e);
    }
    if (ecs::has<display_2d>(e)) {
        gui_display_2d(e);
    }

    if (ecs::has<script_holder>(e)) {
        auto& scripts = ecs::get<script_holder>(e).list;
        for (auto& script : scripts) {
            script->gui_inspector();
        }
    }
}

void gui_window_inspector() {
    ImGui::Begin("Inspector");
    if (!selected_entities.empty()) {
        gui_inspector(selected_entities[0]);
    }
    ImGui::End();
}

}