#include "imgui_module.hpp"

#include <imgui.h>
#include <ek/app/app.hpp>
#include <ek/math/matrix_camera.hpp>
#include <ek/scenex/app/input_controller.hpp>
#include <ek/util/locator.hpp>
#include <ek/util/path.hpp>
#include <ek/system/system.hpp>

#include <ek/graphics/graphics.hpp>

#define SOKOL_IMGUI_IMPL
#define SOKOL_IMGUI_NO_SOKOL_APP

#include <util/sokol_imgui.h>

#define SOKOL_GFX_IMGUI_IMPL

#include <util/sokol_gfx_imgui.h>

namespace ek {

using namespace ek::app;
using namespace ek::graphics;

void reset_keys() {
    auto& io = ImGui::GetIO();
    for (bool& i : io.KeysDown) {
        i = false;
    }
}


imgui_module_t::imgui_module_t() {
    IMGUI_CHECKVERSION();

    sg_imgui_init(&sokol_gfx_gui_state);

    simgui_desc_t desc{
            .color_format = SG_PIXELFORMAT_RGBA8,
            .depth_format = SG_PIXELFORMAT_DEPTH_STENCIL,
            .sample_count = 0,
            .dpi_scale = dpiScale,
            .ini_filename = "imgui.ini",
            .no_default_font = true
    };
    simgui_setup(&desc);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;  // Enable set mouse pos for navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    setup();

    initializeFontTexture();

    ImGui::StyleColorsDark();
}

imgui_module_t::~imgui_module_t() {
    sg_imgui_discard(&sokol_gfx_gui_state);
    simgui_shutdown();
}

#define MAP_KEY_CODE(from, to) io.KeyMap[(to)] = static_cast<int>((from))

void imgui_module_t::setup() {
    auto& io = ImGui::GetIO();
    // Setup back-end capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors |// We can honor GetMouseCursor() values (optional)
                       ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    MAP_KEY_CODE(key_code::Tab, ImGuiKey_Tab);
    MAP_KEY_CODE(key_code::ArrowLeft, ImGuiKey_LeftArrow);
    MAP_KEY_CODE(key_code::ArrowRight, ImGuiKey_RightArrow);
    MAP_KEY_CODE(key_code::ArrowUp, ImGuiKey_UpArrow);
    MAP_KEY_CODE(key_code::ArrowDown, ImGuiKey_DownArrow);
    MAP_KEY_CODE(key_code::PageUp, ImGuiKey_PageUp);
    MAP_KEY_CODE(key_code::PageDown, ImGuiKey_PageDown);
    MAP_KEY_CODE(key_code::Home, ImGuiKey_Home);
    MAP_KEY_CODE(key_code::End, ImGuiKey_End);
    MAP_KEY_CODE(key_code::Insert, ImGuiKey_Insert);
    MAP_KEY_CODE(key_code::Delete, ImGuiKey_Delete);
    MAP_KEY_CODE(key_code::Backspace, ImGuiKey_Backspace);
    MAP_KEY_CODE(key_code::Space, ImGuiKey_Space);
    MAP_KEY_CODE(key_code::Enter, ImGuiKey_Enter);
    MAP_KEY_CODE(key_code::Escape, ImGuiKey_Escape);
    MAP_KEY_CODE(key_code::A, ImGuiKey_A);
    MAP_KEY_CODE(key_code::C, ImGuiKey_C);
    MAP_KEY_CODE(key_code::V, ImGuiKey_V);
    MAP_KEY_CODE(key_code::X, ImGuiKey_X);
    MAP_KEY_CODE(key_code::Y, ImGuiKey_Y);
    MAP_KEY_CODE(key_code::Z, ImGuiKey_Z);

    io.ClipboardUserData = this;
    io.SetClipboardTextFn = [](void* context, const char* text) {
        auto* c = static_cast<imgui_module_t*>(context);
        if (c) {
            c->clipboard_text_ = text;
            // TODO: platform clipboard
            //c->_platform->clipboard.set(text);
        }
    };
    io.GetClipboardTextFn = [](void* context) -> const char* {
        auto* c = static_cast<imgui_module_t*>(context);
        const char* result = nullptr;
        if (c) {
            // TODO: platform clipboard
            //c->clipboard_text_ = c->_platform->clipboard.get();
            result = c->clipboard_text_.c_str();
        }
        return result;
    };
}

void imgui_module_t::initializeFontTexture() {
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = nullptr;
    path_t font_path{"../assets/Cousine-Regular.ttf"};
    auto data = read_file(font_path);
    ImFontConfig fontCfg{};
    fontCfg.FontDataOwnedByAtlas = false;
    if (!data.empty()) {
        font = io.Fonts->AddFontFromMemoryTTF(data.data(), data.size(), 16.0f * dpiScale, &fontCfg);
    }
    font->Scale = 1.0f / dpiScale;

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
    img_desc.content.subimage[0][0].ptr = pixels;
    img_desc.content.subimage[0][0].size = width * height * 4;
    img_desc.label = "sokol-imgui-font";
    fontTexture = sg_make_image(&img_desc);

    ImGui::GetIO().Fonts->TexID = (ImTextureID) (uintptr_t) fontTexture.id;
}

void imgui_module_t::on_event(const event_t& event) {
    auto& io = ImGui::GetIO();
    switch (event.type) {
        case event_type::key_up:
        case event_type::key_down:
        case event_type::key_press: {
            int key = static_cast<int>(event.code);
            if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown)) {
                io.KeysDown[key] = (event.type == event_type::key_down);
            }
            if ((io.KeyShift && !event.shift) || (io.KeyCtrl && !event.ctrl)
                || (io.KeyAlt && !event.alt) || (io.KeySuper && !event.super)) {
                reset_keys();
            }
            io.KeyShift = event.shift;
            io.KeyCtrl = event.ctrl;
            io.KeyAlt = event.alt;
            io.KeySuper = event.super;
        }
            break;

        case event_type::text:
            io.AddInputCharactersUTF8(event.characters.c_str());
            break;

        case event_type::mouse_down:
        case event_type::mouse_up: {
            int button = 0;
            if (event.button == mouse_button::right) {
                button = 1;
            } else if (event.button == mouse_button::other) {
                button = 2;
            }
            io.MouseDown[button] = (event.type == event_type::mouse_down);
        }
            break;
        case event_type::mouse_scroll:
            if (fabs(event.scroll.x) > 0.0) {
                io.MouseWheelH += static_cast<float>(event.scroll.x * 0.1);
            }
            if (fabs(event.scroll.y) > 0.0) {
                io.MouseWheel += static_cast<float>(event.scroll.y * 0.1);
            }
            break;

        case event_type::mouse_move:
            io.MousePos.x = static_cast<float>(event.pos.x / g_app.content_scale);
            io.MousePos.y = static_cast<float>(event.pos.y / g_app.content_scale);
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
    auto cursor = mouse_cursor::parent;
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
                cursor = mouse_cursor::arrow;
                break;
            case ImGuiMouseCursor_Hand:
                cursor = mouse_cursor::button;
                break;
            default:
                break;
        }
        g_app.updateMouseCursor(cursor);
        cursor_visible = true;
    }

    // TODO: cursor visible
    //g_window.hideCursor()->mouse.show(cursorVisible);
}

void imgui_module_t::begin_frame(float dt) {
    auto w = static_cast<int>(g_app.drawable_size.x);
    auto h = static_cast<int>(g_app.drawable_size.y);
    if(w > 0 && h > 0) {
        update_mouse_cursor();
        simgui_new_frame(w, h, dt);
    }
}

void imgui_module_t::end_frame() {
    sg_imgui_draw(&sokol_gfx_gui_state);
    simgui_render();
}

void imgui_module_t::on_frame_completed() {
    auto& io = ImGui::GetIO();
    if (io.KeySuper || io.KeyCtrl) {
        reset_keys();
    }

    auto* ic = try_resolve<input_controller>();
    if (ic) {
        ic->hovered_by_editor_gui = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    }
}

}
