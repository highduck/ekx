#pragma once

#include "EditorWindow.hpp"
#include <ek/debug/LogMessage.hpp>

namespace ek {

struct ConsoleMsg {
    Array<char> text;
    Verbosity verbosity;
    SourceLocation location;
    uint8_t frameHash;
    const char* icon;
    ImU32 iconColor;
};

struct VerbosityFilterInfo {
    Verbosity verbosity = Verbosity::Debug;
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

    void onMessageWrite(const LogMessage& message);

    void execute(const char* cmd);

    void clear();

    Array<ConsoleMsg> messages;
    Array<const char*> commands;
    Array<const char*> candidates;
    Array<char*> history;
    // -1: new line, 0..History.Size-1 browsing history
    int historyPos = 0;

    uint8_t filterMask = 0xFF;
    ImGuiTextFilter textFilter{};
    bool autoScroll = true;
    bool scrollDownRequired = false;

    VerbosityFilterInfo infos[5]{};

    char input[1024] = "";
};

}