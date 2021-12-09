#pragma once

#include "../imgui/imgui.hpp"
#include <pugixml.hpp>

namespace ek {

class EditorWindow {
public:
    String name{};
    String title{};
    bool dirty = false;
    bool opened = true;
    bool fullFrame = false;

    virtual ~EditorWindow() = default;

    virtual void onDraw() {}
    virtual void onLoad(const pugi::xml_node& xml) {(void)xml;}
    virtual void onSave(pugi::xml_node& xml) {(void)xml;}

    void load(const pugi::xml_node& xml) {
        if(name.empty()) return;
        auto node = xml.child(name.c_str());
        if(!node.empty()) {
            opened = node.attribute("opened").as_bool(false);
            onLoad(node);
        }
        dirty = false;
    }

    void save(pugi::xml_node& xml) {
        if(name.empty()) return;
        auto node = xml.child(name.c_str());
        if(node.empty()) {
            node = xml.append_child(name.c_str());
        }
        node.append_attribute("opened").set_value(opened);
        onSave(node);
        dirty = false;
    }

    void show() {
        if(!opened) {
            return;
        }

        int flags = 0;
        if(fullFrame) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
            flags |= ImGuiWindowFlags_NoBackground;
        }

        if(ImGui::Begin(title.c_str(), &opened, flags)) {
            onDraw();
            if(!opened) {
                dirty = true;
            }
        }
        ImGui::End();

        if(fullFrame) {
            ImGui::PopStyleVar();
        }
    }

    void mainMenu() {
        dirty |= ImGui::MenuItem(title.c_str(), nullptr, &opened);
    }
};

}