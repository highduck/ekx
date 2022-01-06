#pragma once

#include "GameWindow.hpp"

namespace ek {

void GameWindow::onDraw() {
    auto& display = Locator::ref<basic_application>().display;
    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const ImVec2 displaySize = ImGui::GetContentRegionAvail();
    if (display.color.id && displaySize.x > 0 && displaySize.y > 0) {
        auto texId = (void*) static_cast<uintptr_t>(display.color.id);

        const float scale = fmin(displaySize.x / display.info.size.x, displaySize.y / display.info.size.y);

        const auto info = sg_query_image_info(display.color);
        const float texCoordX1 = display.info.size.x / (float)info.width;
        const float texCoordY1 = display.info.size.y / (float)info.height;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage(texId, {displayPos.x, displayPos.y},
                           {displayPos.x + scale * display.info.size.x,
                            displayPos.y + scale * display.info.size.y},
                           {0, 0},
                           {texCoordX1, texCoordY1});

        {
            // update size;
            const float k = display.info.dpiScale;
            display.info.destinationViewport = k * rect(displayPos.x, displayPos.y,displaySize.x, displaySize.y);
            display.info.window = vec2(displaySize.x, displaySize.y);
            display.info.size = k * vec2(displaySize.x, displaySize.y);
        }
    }
    auto* ic = Locator::get<input_controller>();
    if (ic) {
        ic->hovered_by_editor_gui = !ImGui::IsWindowHovered(0);
    }
}

void GameWindow::onLoad(const pugi::xml_node& xml) {
    paused = xml.attribute("paused").as_bool(false);
    timeScale = xml.attribute("timeScale").as_float(1.0f);
    profiler = xml.attribute("profile").as_bool(false);
}

void GameWindow::onSave(pugi::xml_node& xml) {
    xml.append_attribute("paused").set_value(paused);
    xml.append_attribute("timeScale").set_value(timeScale);
    xml.append_attribute("profiler").set_value(profiler);
}

}