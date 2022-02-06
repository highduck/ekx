#pragma once

#include "EditorWindow.hpp"
#include <ek/ds/PodArray.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

struct ConsoleMsg {
    char text[1024];
    log_level_t verbosity;
    const char* file;
    int line;
    uint8_t tick;
    const char* icon;
    ImU32 iconColor;
};

struct VerbosityFilterInfo {
    log_level_t verbosity = LOG_LEVEL_DEBUG;
    const char* name = "";
    ImU32 iconColor = 0xFFFFFFFF;
    unsigned count = 0;
    bool show = true;
    const char* icon = "!";
};

class ConsoleWindow : public EditorWindow {
public:

    ConsoleWindow();

    ~ConsoleWindow() override = default;

    void onDraw() override;

    void onMessageWrite(log_msg_t msg);

    void execute(const char* cmd);

    void clear();

    ConsoleMsg messages[1024];
    uint32_t messages_num = 0;
    uint32_t messages_cur = 0;
    PodArray<const char*> commands;
    PodArray<const char*> candidates;
    PodArray<char*> history;
    // -1: new line, 0..History.Size-1 browsing history
    int historyPos = 0;

    uint8_t filterMask = 0xFF;
    ImGuiTextFilter textFilter{};
    bool autoScroll = true;
    bool scrollDownRequired = false;

    VerbosityFilterInfo infos[5]{};

    char input[1024]{0};
};

}