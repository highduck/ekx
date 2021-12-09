#pragma once

#include "GameWindow.hpp"

namespace ek {

void GameWindow::onDraw() {
    auto& display = Locator::ref<basic_application>().display;
    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const ImVec2 displaySize = ImGui::GetContentRegionAvail();
    if (display.color && displaySize.x > 0 && displaySize.y > 0) {
        auto texId = (void*) static_cast<uintptr_t>(display.color->image.id);

        const float scale = fmin(displaySize.x / display.info.size.x, displaySize.y / display.info.size.y);

        const float texCoordX1 = display.info.size.x / static_cast<float>(display.color->desc.width);
        const float texCoordY1 = display.info.size.y / static_cast<float>(display.color->desc.height);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage(texId, {displayPos.x, displayPos.y},
                           {displayPos.x + scale * display.info.size.x,
                            displayPos.y + scale * display.info.size.y},
                           {0, 0},
                           {texCoordX1, texCoordY1});

        {
            // update size;
            const float k = display.info.dpiScale;
            display.info.destinationViewport = Rect2f{
                    k * displayPos.x, k * displayPos.y,
                    k * displaySize.x, k * displaySize.y
            };
            display.info.window.x = displaySize.x;
            display.info.window.y = displaySize.y;
            display.info.size.x = k * displaySize.x;
            display.info.size.y = k * displaySize.y;
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