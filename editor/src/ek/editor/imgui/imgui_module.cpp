#include "imgui_module.hpp"

#include <imgui.h>
#include <graphics/program.hpp>
#include <graphics/texture.hpp>
#include <graphics/buffer_object.hpp>
#include <draw2d/batcher.hpp>
#include <ek/app/app.hpp>
#include <ek/math/matrix_camera.hpp>
#include <graphics/gl_debug.hpp>
#include <scenex/ek/input_controller.h>
#include <ek/util/locator.hpp>
#include <ek/util/path.hpp>
#include <ek/system/system.hpp>

namespace ek {

const GLchar* vertex_shader =
        "#ifdef GL_ES\n"
        "precision highp float;\n"
        "#endif\n"

        "attribute vec2 aPosition;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColorMult;\n"

        "uniform mat4 uModelViewProjection;\n"

        "varying vec2 vTexCoord;\n"
        "varying vec4 vColorMult;\n"

        "void main() {\n"
        "    vTexCoord = aTexCoord;\n"
        "    vColorMult = vec4(aColorMult.xyz * aColorMult.a, aColorMult.a);\n"
        "    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);\n"
        "}\n";

const GLchar* fragment_shader =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"

        "varying vec2 vTexCoord;\n"
        "varying vec4 vColorMult;\n"
        "varying vec4 vColorOffset;\n"

        "uniform sampler2D uImage0;\n"

        "void main() {\n"
        "    vec4 pixel_color = texture2D(uImage0, vTexCoord);\n"
        "    gl_FragColor = pixel_color * vColorMult;\n"
        "}\n";

void reset_keys() {
    auto& io = ImGui::GetIO();
    for (bool& i : io.KeysDown) {
        i = false;
    }
}

void imgui_module_t::on_event(const event_t& event) {
    if (event.type == event_type::key_down
        && event.code == key_code::A
        && event.ctrl && event.shift) {
        enabled_ = !enabled_;
    }

    if (!enabled_) {
        return;
    }

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

void imgui_module_t::render_frame_data(ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    const rect_f viewport_rect{draw_data->DisplayPos.x, draw_data->DisplayPos.y,
                               draw_data->DisplaySize.x, draw_data->DisplaySize.y};
    if (viewport_rect.width <= 0.0f || viewport_rect.height <= 0.0f) {
        return;
    }

    const rect_f framebuffer_rect = viewport_rect
                                    * float2{io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y};
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    gl_check_error();
    glBlendEquation(GL_FUNC_ADD);
    gl_check_error();
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    gl_check_error();
    glDisable(GL_CULL_FACE);
    gl_check_error();
    glDisable(GL_DEPTH_TEST);
    gl_check_error();
    glEnable(GL_SCISSOR_TEST);
    gl_check_error();

    glViewport(static_cast<GLint>(framebuffer_rect.x),
               static_cast<GLint>(framebuffer_rect.y),
               static_cast<GLsizei>(framebuffer_rect.width),
               static_cast<GLsizei>(framebuffer_rect.height));
    gl_check_error();

    auto mvp = ortho_2d(viewport_rect.x, viewport_rect.y, viewport_rect.width, viewport_rect.height);
    program_->use();
    program_->bind_image();
    program_->set_uniform(program_uniforms::mvp, mvp);

    GLint batch_prev_texture = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    gl_check_error();

    ImVec2 pos = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = nullptr;

        vertex_buffer_->upload(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        index_buffer_->upload(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(uint16_t));

        program_->bind_attributes();

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y,
                                          pcmd->ClipRect.z - pos.x,
                                          pcmd->ClipRect.w - pos.y);
                if (clip_rect.x < framebuffer_rect.right()
                    && clip_rect.y < framebuffer_rect.bottom()
                    && clip_rect.z >= framebuffer_rect.x
                    && clip_rect.w >= framebuffer_rect.y) {

                    // Apply scissor/clipping rectangle
                    glScissor(static_cast<GLint>(clip_rect.x),
                              static_cast<GLint>(framebuffer_rect.bottom() - clip_rect.w),
                              static_cast<GLsizei>(clip_rect.z - clip_rect.x),
                              static_cast<GLsizei>(clip_rect.w - clip_rect.y));
                    gl_check_error();

                    const GLint texture_id = (GLuint) (intptr_t) pcmd->TextureId;
                    if (batch_prev_texture != texture_id) {
                        batch_prev_texture = texture_id;
                        // Bind texture
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, (GLuint) texture_id);
                        gl_check_error();
                    }
                    //Draw
                    glDrawElements(GL_TRIANGLES, (GLsizei) pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer_offset);
                    gl_check_error();
                }
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }
    glDisable(GL_SCISSOR_TEST);
    gl_check_error();
}

void imgui_module_t::init_fonts() {
    uint8_t* pixels;
    int width;
    int height;
    ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    auto* pma_pixels = new uint32_t[width * height];
    for (int i = 0; i < width * height; ++i) {
        const uint32_t c = pixels[i];
        pma_pixels[i] = (c << 24u) | (c << 16u) | (c << 8u) | c;
    }
    ImGui::GetIO().Fonts->TexID = reinterpret_cast<ImTextureID>(texture_->handle());
    texture_->upload_pixels(width, height, pma_pixels);
    delete[] pma_pixels;
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
        g_app.set_cursor(cursor);
        cursor_visible = true;
    }

    // TODO: cursor visible
    //g_window.hideCursor()->mouse.show(cursorVisible);
}

imgui_module_t::imgui_module_t() {

    vertex_buffer_ = new buffer_object_t{buffer_type::vertex_buffer, buffer_usage::dynamic_buffer};
    index_buffer_ = new buffer_object_t{buffer_type::index_buffer, buffer_usage::dynamic_buffer};
    texture_ = new texture_t{};
    program_ = new program_t{vertex_shader, fragment_shader};
    program_->vertex = &vertex_minimal_2d::decl;

    /*** imgui setup **/
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;  // Enable set mouse pos for navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    setup();

    /*** set imgui config **/
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
//    ImFont* font = io.Fonts->AddFontDefault();
//    IM_ASSERT(font != NULL);
    float scale_factor = 2.0f;
    ImFont* font = nullptr;
    path_t font_path{"Cousine-Regular.ttf"};
    const char* sdk_root = std::getenv("EKX_ROOT");
    if (sdk_root) {
        font_path = path_t{sdk_root} / "editor/resources" / font_path;
    }
    auto data = read_file(font_path);
    if (!data.empty()) {
        font = io.Fonts->AddFontFromMemoryTTF(data.data(), data.size(), 16.0f * scale_factor);
    }
    if (font == nullptr) {
        ImFontConfig font_cfg;
        font_cfg.OversampleH = 1;
        font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
        font_cfg.SizePixels = 13.0f * scale_factor;
        font = io.Fonts->AddFontDefault(&font_cfg);
    }
    IM_ASSERT(font != nullptr);
    font->Scale = 1.0f / scale_factor;

    ImGui::StyleColorsDark();
    init_fonts();
}

imgui_module_t::~imgui_module_t() {
    delete vertex_buffer_;
    delete index_buffer_;
    delete program_;
    delete texture_;

    ImGui::GetIO().Fonts->TexID = nullptr;
}

void imgui_module_t::begin_frame(float dt) {
    auto w = static_cast<float>(g_app.window_size.x);
    auto h = static_cast<float>(g_app.window_size.y);
    auto fb_w = static_cast<float>(g_app.drawable_size.x);
    auto fb_h = static_cast<float>(g_app.drawable_size.y);

    ImGui::GetIO().DisplaySize = ImVec2(w, h);
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(w > 0 ? (fb_w / w) : 0, h > 0 ? (fb_h / h) : 0);
    ImGui::GetIO().DeltaTime = dt;

    update_mouse_cursor();

    ImGui::NewFrame();
}

void imgui_module_t::end_frame() {
    ImGui::Render();

    if (enabled_) {
        render_frame_data(ImGui::GetDrawData());
    }
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

void imgui_module_t::on_frame_completed() {
    application_listener_t::on_frame_completed();

    auto& io = ImGui::GetIO();
    if (io.KeySuper || io.KeyCtrl) {
        reset_keys();
    }

    auto* ic = try_resolve<scenex::input_controller>();
    if (ic) {
        ic->hovered_by_editor_gui = ImGui::IsAnyWindowHovered() && enabled_;
    }
}

}
