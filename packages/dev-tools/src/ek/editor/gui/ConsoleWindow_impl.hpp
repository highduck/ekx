#pragma once

#include "ConsoleWindow.hpp"

// `system` function
#include <cstdlib>
// `getcwd` function
//#include <libc.h>
#include <stb/stb_sprintf.h>

namespace ek {

// string utilities
void trimRight(char* s) {
    char* it = s + strlen(s);
    while (it > s && it[-1] == ' ') {
        --it;
    }
    *it = '\0';
}

int onConsoleInputCommandCallback(ImGuiInputTextCallbackData* data) {
    auto* con = static_cast<ConsoleWindow*>(data->UserData);
    auto& commands = con->commands;
    auto& candidates = con->candidates;
    //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
    switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf) {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

            // Build a list of candidates
            candidates.clear();
            for (auto& command: commands) {
                if (strncasecmp(command, word_start, (int) (word_end - word_start)) == 0) {
                    candidates.push_back(command);
                }
            }

            if (candidates.empty()) {
                // No match
                EK_INFO("No match for \"%.*s\"!\n", (int) (word_end - word_start), word_start);
            } else if (candidates.size() == 1) {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((int) (word_start - data->Buf), (int) (word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            } else {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int) (word_end - word_start);
                for (;;) {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.size() && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0) {
                    data->DeleteChars((int) (word_start - data->Buf), (int) (word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                EK_INFO("Possible matches:");
                for (auto& candidate: candidates) {
                    EK_INFO("- %s", candidate);
                }
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory: {
            // Example of HISTORY
            const int prev_history_pos = con->historyPos;
            if (data->EventKey == ImGuiKey_UpArrow) {
                if (con->historyPos == -1) {
                    con->historyPos = static_cast<int>(con->history.size()) - 1;
                } else if (con->historyPos > 0) {
                    --con->historyPos;
                }
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (con->historyPos != -1) {
                    if (++con->historyPos >= con->history.size()) {
                        con->historyPos = -1;
                    }
                }
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != con->historyPos) {
                const char* history_str = (con->historyPos >= 0) ? con->history[con->historyPos] : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
    }
    return 0;
}

void ConsoleWindow::onDraw() {
    unsigned filterMask = 0;
    char tmpBuffer[64];
    for (int i = 0; i < 5; ++i) {
        auto& info = infos[i];
        unsigned count = 0;
        for (auto& msg: messages) {
            if (msg.verbosity == info.verbosity) {
                ++count;
            }
        }
        info.count = count;
        sprintf(tmpBuffer, "%s %u###console_verbosity_%i", info.icon, count, i);
        ImGui::PushStyleColor(ImGuiCol_Button, info.show ? 0x44FF7722 : 0x0);
        ImGui::PushStyleColor(ImGuiCol_Text, info.show ? info.iconColor : 0x3FFFFFFF);
        if (ImGui::Button(tmpBuffer)) {
            info.show = !info.show;
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Toggle %s Verbosity", info.name);
        }
        if (info.show) {
            filterMask |= 1 << i;
        }
        ImGui::SameLine(0, 1);
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_TRASH, false, "Clear All")) {
        clear();
    }
    ImGui::SameLine(0, 0);
    if (ImGui::ToolbarButton(ICON_FA_ANGLE_DOUBLE_DOWN, autoScroll, "Scroll to End")) {
        autoScroll = !autoScroll;
        if (autoScroll) {
            scrollDownRequired = true;
        }
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::TextUnformatted(ICON_FA_SEARCH);
    ImGui::SameLine();
    textFilter.Draw("##logs_filter", 100.0f);
    if (textFilter.IsActive()) {
        ImGui::SameLine(0, 0);
        if (ImGui::ToolbarButton(ICON_FA_TIMES_CIRCLE, false, "Clear Filter")) {
            textFilter.Clear();
        }
    }

    auto logListSize = ImGui::GetContentRegionAvail();
    logListSize.y -= 30;
    ImGui::BeginChild("log_lines", logListSize);
    for (auto& msg: messages) {
        const auto* text = msg.text.data();
        if (textFilter.IsActive() && !textFilter.PassFilter(text)) {
            continue;
        }
        if (!!((1 << (int) msg.verbosity) & filterMask) && textFilter.PassFilter(text)) {
            ImGui::PushStyleColor(ImGuiCol_Text, msg.iconColor);
            ImGui::PushID(&msg);
            if (ImGui::Selectable(msg.icon)) {
                // Tools -> Create Command-line Launcher...
                char buf[512];
                stbsp_sprintf(buf, "clion --line %u ../%s", msg.line, msg.file);
                system(buf);
            }
            ImGui::PopID();
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s:%u", msg.file, msg.line);
            }

            ImGui::SameLine(0, 10);
            ImGui::TextUnformatted(text);
        }
    }
    if (scrollDownRequired || (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
        ImGui::SetScrollHereY(1.0f);
        scrollDownRequired = false;
    }
    ImGui::EndChild();

    ImGui::Separator();
    int flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion |
                ImGuiInputTextFlags_CallbackHistory;
    ImGui::TextUnformatted(ICON_FA_TERMINAL);
    ImGui::SameLine();
    bool reclaimFocus = false;
    if (ImGui::InputText("###" "WindowConsole_InputText", input, 1024, flags,
                         onConsoleInputCommandCallback, this)) {
        trimRight(input);
        if (input[0] != '\0') {
            execute(input);
        }
        input[0] = '\0';
        reclaimFocus = true;
    }
    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
    if (reclaimFocus) {
        // Auto focus previous widget
        ImGui::SetKeyboardFocusHere(-1);
    }
}

void ConsoleWindow::onMessageWrite(log_msg_t msg) {
    if (msg.text == nullptr || msg.text[0] == '\0') {
        return;
    }

    auto len = (uint32_t) strnlen(msg.text, 1024);

    const char* icon = "";
    ImU32 iconColor = 0xFFFFFFFF;
    for (int i = 0; i < 5; ++i) {
        if (infos[i].verbosity == msg.level) {
            icon = infos[i].icon;
            iconColor = infos[i].iconColor;
        }
    }

    ConsoleMsg cmsg{
            Array<char>(len + 1),
            (log_level_t) msg.level,
            msg.file,
            msg.line,
            (uint8_t) msg.frame,
            icon,
            iconColor
    };

    cmsg.text.resize(len + 1);
    memcpy(cmsg.text.data(), msg.text, cmsg.text.size());
    messages.emplace_back(std::move(cmsg));

    if (autoScroll) {
        scrollDownRequired = true;
    }
}

inline ConsoleWindow* _consoleWindow = nullptr;

inline void logToConsoleWindow(log_msg_t msg) {
    if (_consoleWindow) {
        _consoleWindow->onMessageWrite(msg);
    }
}

ConsoleWindow::ConsoleWindow() {

    name = "ConsoleWindow";
    title = ICON_FA_LAPTOP_CODE " Console###ConsoleWindow";

    _consoleWindow = this;
    log_add_sink(logToConsoleWindow);

    infos[0].verbosity = LOG_LEVEL_TRACE;
    infos[0].icon = ICON_FA_GLASSES;
    infos[0].name = "Trace";
    infos[0].iconColor = 0xFF999999;

    infos[1].verbosity = LOG_LEVEL_DEBUG;
    infos[1].icon = ICON_FA_BUG;
    infos[1].name = "Debug";
    infos[1].iconColor = 0xFFAAAA00;

    infos[2].verbosity = LOG_LEVEL_INFO;
    infos[2].icon = ICON_FA_INFO_CIRCLE;
    infos[2].name = "Info";
    infos[2].iconColor = 0xFFFFFFFF;

    infos[3].verbosity = LOG_LEVEL_WARN;
    infos[3].icon = ICON_FA_EXCLAMATION_TRIANGLE;
    infos[3].name = "Warning";
    infos[3].iconColor = 0xFF44BBFF;

    infos[4].verbosity = LOG_LEVEL_ERROR;
    infos[4].icon = ICON_FA_BAN;
    infos[4].name = "Error";
    infos[4].iconColor = 0xFF4444DD;

    commands.push_back("CLEAR");
    commands.push_back("HELP");
    commands.push_back("HISTORY");
}

void ConsoleWindow::execute(const char* cmd) {
    EK_INFO("$ %s", cmd);

    // Insert into history. First find match and delete it so it can be pushed to the back.
    // This isn't trying to be smart or optimal.
    historyPos = -1;
    for (int i = static_cast<int>(history.size()) - 1; i >= 0; --i) {
        if (strcasecmp(history[i], cmd) == 0) {
            free(history[i]);
            history.eraseAt(i);
            break;
        }
    }
    auto cmdLen = strlen(cmd);
    auto* cmdCopy = (char*) malloc(cmdLen + 1);
    memcpy(cmdCopy, cmd, cmdLen + 1);
    history.push_back(cmdCopy);
    // Process command
    if (strcasecmp(cmd, "CLEAR") == 0) {
        clear();
    } else if (strcasecmp(cmd, "HELP") == 0) {
        EK_INFO("Commands:");
        for (auto* command: commands) {
            EK_INFO("- %s", command);
        }
    } else if (strcasecmp(cmd, "HISTORY") == 0) {
        int first = static_cast<int>(history.size()) - 10;
        for (int i = first > 0 ? first : 0; i < history.size(); ++i) {
            EK_INFO("%3d: %s", i, history[i]);
        }
    } else {
        EK_INFO("Unknown command: %s", cmd);
    }

    // On command input, we scroll to bottom even if AutoScroll==false
    scrollDownRequired = true;
}

void ConsoleWindow::clear() {
    messages.clear();
}

}