#include "imgui_module.hpp"

#include <imgui.h>
#include <graphics/program.hpp>
#include <graphics/texture.hpp>
#include <graphics/buffer_object.hpp>
#include <draw2d/batcher.hpp>
#include <platform/Window.h>
#include <platform/static_resources.hpp>
#include <ek/math/matrix_camera.hpp>
#include <graphics/gl_debug.hpp>
#include <scenex/ek/input_controller.h>
#include <ek/locator.hpp>

using namespace ek;

namespace scenex {

const GLchar* vertex_shader =
        "#ifdef GL_ES\n"
        "precision highp float;\n"
        "#endif\n"
        "attribute vec2 Position;\n"
        "attribute vec2 UV;\n"
        "attribute vec4 Color;\n"
        "uniform mat4 ProjMtx;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "void main() {\n"
        "    Frag_UV = UV;\n"
        // todo: avoid mult
        "    Frag_Color = vec4(Color.xyz * Color.a, Color.a);\n"
        "    gl_Position = ProjMtx * vec4(Position.xy, 0.0, 1.0);\n"
        "}\n";

const GLchar* fragment_shader =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "uniform sampler2D Texture;\n"
        "void main() {\n"
        "   vec4 pixel_color = texture2D(Texture, Frag_UV);\n"
        "   pixel_color *= Frag_Color;\n"
        // gl_FragColor = pixelColor + vColorOffset * pixelColor.wwww;
        "   gl_FragColor = pixel_color;\n"
        "}\n";

void reset_keys() {
    auto& io = ImGui::GetIO();
    for (bool& i : io.KeysDown) {
        i = false;
    }
}

void imgui_module_t::onKeyEvent(const KeyEvent& event) {
    auto& io = ImGui::GetIO();
    int key = static_cast<int>(event.code);

    if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown)) {
        io.KeysDown[key] = (event.type == KeyEvent::Type::Down);
    }
    if ((io.KeyShift && !event.shift) || (io.KeyCtrl && !event.ctrl)
        || (io.KeyAlt && !event.alt) || (io.KeySuper && !event.super)) {
        reset_keys();
    }
    io.KeyShift = event.shift;
    io.KeyCtrl = event.ctrl;
    io.KeyAlt = event.alt;
    io.KeySuper = event.super;

    if (event.type == KeyEvent::Type::Down
        && event.code == KeyEvent::Code::A
        && event.ctrl && event.shift) {
        enabled_ = !enabled_;
    }
}

void imgui_module_t::on_text_event(const text_event_t& event) {
    ImGui::GetIO().AddInputCharactersUTF8(event.characters.c_str());
    //return true;
}

void imgui_module_t::onMouseEvent(const MouseEvent& event) {
    if (!enabled_) {
        return;
    }
    auto& io = ImGui::GetIO();
    if (event.type == MouseEvent::Type::Down || event.type == MouseEvent::Type::Up) {
        int button = 0;
        if (event.button == MouseEvent::Button::Right) {
            button = 1;
        } else if (event.button == MouseEvent::Button::Other) {
            button = 2;
        }
        io.MouseDown[button] = (event.type == MouseEvent::Type::Down);
    } else if (event.type == MouseEvent::Type::Scroll) {
        if (fabsf(event.scrollX) > 0.0) {
            io.MouseWheelH += event.scrollX * 0.1f;
        }
        if (fabsf(event.scrollY) > 0.0) {
            io.MouseWheel += event.scrollY * 0.1f;
        }
    } else if (event.type == MouseEvent::Type::Move) {
        io.MousePos.x = event.x / gWindow.scaleFactor;
        io.MousePos.y = event.y / gWindow.scaleFactor;
    }
}

void imgui_module_t::onTouchEvent(const TouchEvent&) {

}

void imgui_module_t::onAppEvent(const AppEvent&) {

}

void imgui_module_t::onDrawFrame() {

}

void imgui_module_t::enable_vertex_attributes() {
    glEnableVertexAttribArray((GLuint) locations_.position);
    gl_check_error();

    glEnableVertexAttribArray((GLuint) locations_.uv);
    gl_check_error();

    glEnableVertexAttribArray((GLuint) locations_.color);
    gl_check_error();

    glVertexAttribPointer((GLuint) locations_.position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                          (GLvoid*) IM_OFFSETOF(ImDrawVert, pos));
    gl_check_error();

    glVertexAttribPointer((GLuint) locations_.uv, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                          (GLvoid*) IM_OFFSETOF(ImDrawVert, uv));
    gl_check_error();

    glVertexAttribPointer((GLuint) locations_.color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),
                          (GLvoid*) IM_OFFSETOF(ImDrawVert, col));
    gl_check_error();
}

// OpenGL3 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
void imgui_module_t::render_frame_data(ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    const float fb_width = draw_data->DisplaySize.x * io.DisplayFramebufferScale.x;
    const float fb_height = draw_data->DisplaySize.y * io.DisplayFramebufferScale.y;
    if (fb_width <= 0.0f || fb_height <= 0.0f) {
        return;
    }
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

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
    glViewport(0, 0, (GLsizei) fb_width, (GLsizei) fb_height);

    gl_check_error();
    auto mvp = ortho_2d(
            draw_data->DisplayPos.x,
            draw_data->DisplayPos.y,
            draw_data->DisplaySize.x,
            draw_data->DisplaySize.y
    );
    program_->use();
    glUniform1i(locations_.tex, 0);
    gl_check_error();
    glUniformMatrix4fv(locations_.proj, 1, GL_FALSE, mvp.m);
    gl_check_error();
#ifdef GL_SAMPLER_BINDING
    //        glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
//        glCheckError();
#endif

//#ifndef WEBGL
// Recreate the VAO every time
//     (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
//    GLuint vao_handle = 0;
//    glGenVertexArrays(1, &vao_handle);
//    glBindVertexArray(vao_handle);
//#endif
//    GLuint vao = 0;
//    glGenVertexArrays(1, &vao);
//    glCheckError();
//    glBindVertexArray(vao);
//    glCheckError();


    GLint batch_prev_texture = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    gl_check_error();
    // Draw
    ImVec2 pos = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = nullptr;

        vertex_buffer_->upload(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        index_buffer_->upload(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(uint16_t));

        enable_vertex_attributes();

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y,
                                          pcmd->ClipRect.z - pos.x,
                                          pcmd->ClipRect.w - pos.y);
                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f &&
                    clip_rect.w >= 0.0f) {
                    // Apply scissor/clipping rectangle
                    glScissor((int) clip_rect.x, (int) (fb_height - clip_rect.w), (int) (clip_rect.z - clip_rect.x),
                              (int) (clip_rect.w - clip_rect.y));
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
//#ifndef WEBGL
//    glBindVertexArray(0);
//    glDeleteVertexArrays(1, &vao);
//#endif
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

void imgui_module_t::init_program() {
    GLuint handle = program_->handle();
    locations_.tex = glGetUniformLocation(handle, "Texture");
    gl_check_error();
    locations_.proj = glGetUniformLocation(handle, "ProjMtx");
    gl_check_error();
    locations_.position = glGetAttribLocation(handle, "Position");
    gl_check_error();
    locations_.uv = glGetAttribLocation(handle, "UV");
    gl_check_error();
    locations_.color = glGetAttribLocation(handle, "Color");
    gl_check_error();
}

const char* imgui_clipboard_get(void* context) {
    auto* c = static_cast<imgui_module_t*>(context);
    if (c) {
        // TODO: platform clipboard
        //c->clipboard_text_ = c->_platform->clipboard.get();
        return c->clipboard_text_.c_str();
    }
    return nullptr;
}

void imgui_clipboard_set(void* context, const char* text) {
    auto* c = static_cast<imgui_module_t*>(context);
    if (c) {
        c->clipboard_text_ = text;
        // TODO: platform clipboard
//        c->_platform->clipboard.set(text);
    }
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
//    bool ImGuiModule::processEvent(SDL_Event* event) {
//        ImGuiIO& io = ImGui::GetIO();
//        switch (event->type) {
//            case SDL_MOUSEWHEEL: {
//                if (event->wheel.x > 0) io.MouseWheelH += 1;
//                if (event->wheel.x < 0) io.MouseWheelH -= 1;
//                if (event->wheel.y > 0) io.MouseWheel += 1;
//                if (event->wheel.y < 0) io.MouseWheel -= 1;
//                return true;
//            }
//            case SDL_MOUSEBUTTONDOWN: {
//                if (event->button.button == SDL_BUTTON_LEFT) _MousePressed[0] = true;
//                if (event->button.button == SDL_BUTTON_RIGHT) _MousePressed[1] = true;
//                if (event->button.button == SDL_BUTTON_MIDDLE) _MousePressed[2] = true;
//                return true;
//            }
//            default:
//                break;
//        }
//
//        return false;
//    }

void imgui_module_t::update_mouse_state() {
//    ImGuiIO& io = ImGui::GetIO();
//    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
//    if (io.WantSetMousePos) {
//        // TODO: force set mouse position
//        //gWindow.->mouse.position((int)io.MousePos.x, (int)io.MousePos.y);
//    } else {
//        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
//    }
//
//    int mx, my;
//    Uint32 mouse_buttons = SDL_GetMouseState(&mx, &my);
//    io.MouseDown[0] = _MousePressed[0] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) !=
//                                          0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
//    io.MouseDown[1] = _MousePressed[1] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
//    io.MouseDown[2] = _MousePressed[2] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
//    _MousePressed[0] = _MousePressed[1] = _MousePressed[2] = false;
//
#ifndef __EMSCRIPTEN__
//    SDL_Window* focused_window = SDL_GetKeyboardFocus();
//    if (sdl_window == focused_window) {
//        // SDL_GetMouseState() gives mouse position seemingly based on the last window entered/focused(?)
//        // The creation of a new windows at runtime and SDL_CaptureMouse both seems to severely mess up with that, so we retrieve that position globally.
//        int wx, wy;
//        SDL_GetWindowPosition(focused_window, &wx, &wy);
//        SDL_GetGlobalMouseState(&mx, &my);
//        mx -= wx;
//        my -= wy;
//        io.MousePos = ImVec2((float)mx, (float)my);
//    }

    // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger the OS window resize cursor.
    // The function is only supported from SDL 2.0.4 (released Jan 2016)

    //bool any_mouse_button_down = ImGui::IsAnyMouseDown();
    // TODO: capture mouse
//    SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
#else
    // TODO: capture mouse
//    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_INPUT_FOCUS)
//            io.MousePos = ImVec2((float)mx, (float)my);
#endif
}

void imgui_module_t::update_mouse_cursor() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) {
        return;
    }

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    MouseCursor cursor = MouseCursor::Auto;
    bool cursorVisible = true;
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        cursorVisible = false;
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
                cursor = MouseCursor::Auto;
                break;
            case ImGuiMouseCursor_Hand:
                cursor = MouseCursor::Button;
                break;
            default:
                break;
        }
        gWindow.setCursor(cursor);
        cursorVisible = true;
    }

    // TODO: cursor visible
    //gWindow.hideCursor()->mouse.show(cursorVisible);
}

imgui_module_t::imgui_module_t() {

    vertex_buffer_ = new buffer_object_t{buffer_type::vertex_buffer, buffer_usage::dynamic_buffer};
    index_buffer_ = new buffer_object_t{buffer_type::index_buffer, buffer_usage::dynamic_buffer};
    texture_ = new texture_t{};
    program_ = new program_t{vertex_shader, fragment_shader};

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
    auto data = get_resource_content("assets/Cousine-Regular.ttf");
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
    init_program();
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
    auto w = static_cast<float>(gWindow.windowSize.width);
    auto h = static_cast<float>(gWindow.windowSize.height);
    auto fb_w = static_cast<float>(gWindow.backBufferSize.width);
    auto fb_h = static_cast<float>(gWindow.backBufferSize.height);

    ImGui::GetIO().DisplaySize = ImVec2(w, h);
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(w > 0 ? (fb_w / w) : 0, h > 0 ? (fb_h / h) : 0);
    ImGui::GetIO().DeltaTime = dt;

    update_mouse_state();
    update_mouse_cursor();

    ImGui::NewFrame();
}

void imgui_module_t::end_frame() {
    ImGui::Render();

    if (enabled_) {
        render_frame_data(ImGui::GetDrawData());
    }

//    ImGui::GetIO().MouseDown[0] = false;
//    ImGui::GetIO().MouseDown[1] = false;
//    ImGui::GetIO().MouseDown[2] = false;
}

void imgui_module_t::setup() {
    auto& io = ImGui::GetIO();
    // Setup back-end capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)


    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = static_cast<int>(KeyEvent::Code::Tab);
    io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(KeyEvent::Code::ArrowLeft);
    io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(KeyEvent::Code::ArrowRight);
    io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(KeyEvent::Code::ArrowUp);
    io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(KeyEvent::Code::ArrowDown);
    io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(KeyEvent::Code::PageUp);
    io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(KeyEvent::Code::PageDown);
    io.KeyMap[ImGuiKey_Home] = static_cast<int>(KeyEvent::Code::Home);
    io.KeyMap[ImGuiKey_End] = static_cast<int>(KeyEvent::Code::End);
    io.KeyMap[ImGuiKey_Insert] = static_cast<int>(KeyEvent::Code::Insert);
    io.KeyMap[ImGuiKey_Delete] = static_cast<int>(KeyEvent::Code::Delete);
    io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(KeyEvent::Code::Backspace);
    io.KeyMap[ImGuiKey_Space] = static_cast<int>(KeyEvent::Code::Space);
    io.KeyMap[ImGuiKey_Enter] = static_cast<int>(KeyEvent::Code::Enter);
    io.KeyMap[ImGuiKey_Escape] = static_cast<int>(KeyEvent::Code::Escape);
    io.KeyMap[ImGuiKey_A] = static_cast<int>(KeyEvent::Code::A);
    io.KeyMap[ImGuiKey_C] = static_cast<int>(KeyEvent::Code::C);
    io.KeyMap[ImGuiKey_V] = static_cast<int>(KeyEvent::Code::V);
    io.KeyMap[ImGuiKey_X] = static_cast<int>(KeyEvent::Code::X);
    io.KeyMap[ImGuiKey_Y] = static_cast<int>(KeyEvent::Code::Y);
    io.KeyMap[ImGuiKey_Z] = static_cast<int>(KeyEvent::Code::Z);

    io.ClipboardUserData = this;
    io.SetClipboardTextFn = imgui_clipboard_set;
    io.GetClipboardTextFn = imgui_clipboard_get;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        io.ImeWindowHandle = wmInfo.info.win.window;
#endif

}

void imgui_module_t::on_frame_completed() {
    Listener::on_frame_completed();

    auto& io = ImGui::GetIO();
    if (io.KeySuper || io.KeyCtrl) {
        reset_keys();
    }

    auto* ic = ek::try_resolve<input_controller>();
    if (ic) {
        ic->hovered_by_editor_gui = ImGui::IsAnyWindowHovered() && enabled_;
    }
}

}
