#include <ek/editor/imgui/imgui.hpp>

#include "ImGuiIntegration.hpp"
#include <ek/app.h>
#include <IconsFontAwesome5.h>

#include <ek/gfx.h>

#define SOKOL_IMGUI_IMPL
#define SOKOL_IMGUI_NO_SOKOL_APP

#include <util/sokol_imgui.h>

#define SOKOL_GFX_IMGUI_IMPL

#include <util/sokol_gfx_imgui.h>

namespace ek {

void reset_keys() {
    auto& io = ImGui::GetIO();
    for (bool& i : io.KeysDown) {
        i = false;
    }
}

ImGuiIntegration::ImGuiIntegration() {
    IMGUI_CHECKVERSION();

    const sg_imgui_desc_t imgui_desc = {0};
    sg_imgui_init(&sokol_gfx_gui_state, &imgui_desc);

    simgui_desc_t desc{};
    desc.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    desc.sample_count = 0;
    desc.ini_filename = "imgui.ini";
    desc.no_default_font = true;
    simgui_setup(&desc);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;  // Enable set mouse pos for navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    setup();

    init_font_image();

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Alpha = 0.8f;

    ImPlot::CreateContext();
}

ImGuiIntegration::~ImGuiIntegration() {
    ImPlot::DestroyContext();
    sg_imgui_discard(&sokol_gfx_gui_state);
    simgui_shutdown();
}

#define MAP_KEY_CODE(from, to) io.KeyMap[(to)] = (int)((from))

void ImGuiIntegration::setup() {
    auto& io = ImGui::GetIO();
    // Setup back-end capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors |// We can honor GetMouseCursor() values (optional)
                       ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    MAP_KEY_CODE(EK_KEYCODE_TAB, ImGuiKey_Tab);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_LEFT, ImGuiKey_LeftArrow);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_RIGHT, ImGuiKey_RightArrow);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_UP, ImGuiKey_UpArrow);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_DOWN, ImGuiKey_DownArrow);
    MAP_KEY_CODE(EK_KEYCODE_PAGEUP, ImGuiKey_PageUp);
    MAP_KEY_CODE(EK_KEYCODE_PAGEDOWN, ImGuiKey_PageDown);
    MAP_KEY_CODE(EK_KEYCODE_HOME, ImGuiKey_Home);
    MAP_KEY_CODE(EK_KEYCODE_END, ImGuiKey_End);
    MAP_KEY_CODE(EK_KEYCODE_INSERT, ImGuiKey_Insert);
    MAP_KEY_CODE(EK_KEYCODE_DELETE, ImGuiKey_Delete);
    MAP_KEY_CODE(EK_KEYCODE_BACKSPACE, ImGuiKey_Backspace);
    MAP_KEY_CODE(EK_KEYCODE_SPACE, ImGuiKey_Space);
    MAP_KEY_CODE(EK_KEYCODE_ENTER, ImGuiKey_Enter);
    MAP_KEY_CODE(EK_KEYCODE_ESCAPE, ImGuiKey_Escape);
    MAP_KEY_CODE(EK_KEYCODE_A, ImGuiKey_A);
    MAP_KEY_CODE(EK_KEYCODE_C, ImGuiKey_C);
    MAP_KEY_CODE(EK_KEYCODE_V, ImGuiKey_V);
    MAP_KEY_CODE(EK_KEYCODE_X, ImGuiKey_X);
    MAP_KEY_CODE(EK_KEYCODE_Y, ImGuiKey_Y);
    MAP_KEY_CODE(EK_KEYCODE_Z, ImGuiKey_Z);

    io.ClipboardUserData = this;
    io.SetClipboardTextFn = [](void* context, const char* text) {
        auto* c = static_cast<ImGuiIntegration*>(context);
        if (c) {
            c->clipboard_text_ = text;
            // TODO: platform clipboard
            //c->_platform->clipboard.set(text);
        }
    };
    io.GetClipboardTextFn = [](void* context) -> const char* {
        auto* c = static_cast<ImGuiIntegration*>(context);
        const char* result = nullptr;
        if (c) {
            // TODO: platform clipboard
            //c->clipboard_text_ = c->_platform->clipboard.get();
            result = c->clipboard_text_.c_str();
        }
        return result;
    };
}

void addFontWithIcons(const char* filePath, float dpiScale) {
    ImGuiIO& io = ImGui::GetIO();
    float pxSize = (13.0f * dpiScale) * 96.0f / 72.0f;
    float fontSizeScaled = pxSize;
    // MD
    //float iconSizeScaled = pxSize * 1.1f;
    // FA
    float iconSizeScaled = pxSize * 0.75f;
    float fontScale = 1.0f / dpiScale;
    {
        ImFontConfig fontCfg{};
        fontCfg.OversampleH = 1;
        fontCfg.OversampleV = 1;
        fontCfg.PixelSnapH = true;
        fontCfg.FontDataOwnedByAtlas = false;
        ImFont* font = io.Fonts->AddFontFromFileTTF(filePath, fontSizeScaled, &fontCfg);
        font->Scale = fontScale;
    }

    static const ImWchar iconsRange[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    {
        ImFontConfig fontCfg{};
        fontCfg.OversampleH = 1;
        fontCfg.OversampleV = 1;
        fontCfg.PixelSnapH = true;
        fontCfg.FontDataOwnedByAtlas = false;
        fontCfg.GlyphMinAdvanceX = fontSizeScaled; // <-- _FONT_SizeScaled!
        // MD
        //fontCfg.GlyphOffset.y = floorf(0.27f * iconSizeScaled);
        // FA
        fontCfg.GlyphOffset.y = floorf(0.05f * iconSizeScaled);
        fontCfg.MergeMode = true;
        ImFont* font = io.Fonts->AddFontFromFileTTF("dev/" FONT_ICON_FILE_NAME_FAS,
                                                    iconSizeScaled, &fontCfg, iconsRange);
        font->Scale = fontScale;
    }
}

void ImGuiIntegration::init_font_image() {
    ImGuiIO& io = ImGui::GetIO();
    addFontWithIcons("dev/sf-pro-text-regular.ttf", dpiScale);
    addFontWithIcons("dev/sf-mono-text-regular.ttf", dpiScale);

    uint8_t* pixels;
    int width;
    int height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    sg_image_desc img_desc;
    memset(&img_desc, 0, sizeof(img_desc));
    img_desc.width = width;
    img_desc.height = height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.min_filter = SG_FILTER_LINEAR;
    img_desc.mag_filter = SG_FILTER_LINEAR;
    img_desc.data.subimage[0][0].ptr = pixels;
    img_desc.data.subimage[0][0].size = width * height * 4;
    img_desc.label = "sokol-imgui-font";
    font_image = sg_make_image(&img_desc);

    ImGui::GetIO().Fonts->TexID = (ImTextureID) (uintptr_t) font_image.id;
}

void ImGuiIntegration::on_event(const ek_app_event& event) {
    auto& io = ImGui::GetIO();
    switch (event.type) {
        case EK_APP_EVENT_KEY_UP:
        case EK_APP_EVENT_KEY_DOWN:
        case EK_APP_EVENT_KEY_PRESS: {
            int key = static_cast<int>(event.key.code);
            if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown)) {
                io.KeysDown[key] = (event.type == EK_APP_EVENT_KEY_DOWN);
            }

            // update modifier keys
            const bool isShift = (event.key.modifiers & EK_KEY_MOD_SHIFT) != 0;
            const bool isControl = (event.key.modifiers & EK_KEY_MOD_CONTROL) != 0;
            const bool isAlt = (event.key.modifiers & EK_KEY_MOD_ALT) != 0;
            const bool isSuper = (event.key.modifiers & EK_KEY_MOD_SUPER) != 0;
            if ((io.KeyShift && !isShift) || (io.KeyCtrl && !isControl) || (io.KeyAlt && !isAlt) ||
                (io.KeySuper && !isSuper)) {
                // need to reset key states when any of meta keys disabled
                reset_keys();
            }
            io.KeyShift = isShift;
            io.KeyCtrl = isControl;
            io.KeyAlt = isAlt;
            io.KeySuper = isSuper;
        }
            break;

        case EK_APP_EVENT_TEXT:
            if (*event.text.data /* not empty */) {
                io.AddInputCharactersUTF8(event.text.data);
            }
            break;

        case EK_APP_EVENT_MOUSE_DOWN:
        case EK_APP_EVENT_MOUSE_UP: {
            int button = 0;
            if (event.mouse.button == EK_MOUSE_BUTTON_RIGHT) {
                button = 1;
            } else if (event.mouse.button == EK_MOUSE_BUTTON_OTHER) {
                button = 2;
            }
            io.MouseDown[button] = (event.type == EK_APP_EVENT_MOUSE_DOWN);
        }
            break;
        case EK_APP_EVENT_WHEEL:
            if (fabs(event.wheel.x) > 0.0f) {
                io.MouseWheelH += event.wheel.x * 0.1f;
            }
            if (fabs(event.wheel.y) > 0.0f) {
                io.MouseWheel += event.wheel.y * 0.1f;
            }
            break;

        case EK_APP_EVENT_MOUSE_MOVE:
            io.MousePos.x = event.mouse.x / ek_app.viewport.scale;
            io.MousePos.y = event.mouse.y / ek_app.viewport.scale;
            break;

        case EK_APP_EVENT_PAUSE:
        case EK_APP_EVENT_RESUME:
            break;

        default:
            break;
    }

}

void update_mouse_cursor() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) {
        return;
    }

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    auto cursor = EK_MOUSE_CURSOR_PARENT;
    bool cursor_visible = true;
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        cursor_visible = false;
    } else {
        // Show OS mouse cursor
        switch (imgui_cursor) {
            case ImGuiMouseCursor_Arrow:
            case ImGuiMouseCursor_TextInput:
            case ImGuiMouseCursor_ResizeAll:
            case ImGuiMouseCursor_ResizeNS:
            case ImGuiMouseCursor_ResizeEW:
            case ImGuiMouseCursor_ResizeNESW:
            case ImGuiMouseCursor_ResizeNWSE:
                cursor = EK_MOUSE_CURSOR_ARROW;
                break;
            case ImGuiMouseCursor_Hand:
                cursor = EK_MOUSE_CURSOR_BUTTON;
                break;
            default:
                break;
        }
        ek_app_set_mouse_cursor(cursor);
        cursor_visible = true;
    }

    // TODO: cursor visible
    //g_window.hideCursor()->mouse.show(cursorVisible);
}

void ImGuiIntegration::begin_frame(float dt) {
    auto w = (int)ek_app.viewport.width;
    auto h = (int)ek_app.viewport.height;
    if (w > 0 && h > 0) {
        update_mouse_cursor();
        simgui_frame_desc_t desc;
        desc.width = w;
        desc.height = h;
        desc.delta_time = dt;
        desc.dpi_scale = dpiScale;
        simgui_new_frame(&desc);
    }
}

void ImGuiIntegration::end_frame() {
    sg_imgui_draw(&sokol_gfx_gui_state);
    simgui_render();
}

void ImGuiIntegration::on_frame_completed() {
    auto& io = ImGui::GetIO();
    if (io.KeySuper || io.KeyCtrl) {
        reset_keys();
    }
}

}
