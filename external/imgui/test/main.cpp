#include <stdio.h>
#include <stdlib.h>
#include <imgui.h>
#include <fonts/IconsFontAwesome5.h>
#include <fonts/IconsMaterialDesign.h>
#include <imgui_memory_editor/imgui_memory_editor.h>

int main(int argc, char* argv[]) {

    printf("- Font Awesome 5 Regular: %s\n", FONT_ICON_FILE_NAME_FAR);
    printf("- Font Awesome 5 Solid: %s\n", FONT_ICON_FILE_NAME_FAS);
    printf("- Material Icons Regular: %s\n", FONT_ICON_FILE_NAME_MD);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Build atlas
    unsigned char* tex_pixels = NULL;
    int tex_w, tex_h;
    io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

    for (int n = 0; n < 20; n++)
    {
        printf("NewFrame() %d\n", n);
        io.DisplaySize = ImVec2(1920, 1080);
        io.DeltaTime = 1.0f / 60.0f;
        ImGui::NewFrame();

        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::ShowDemoWindow(NULL);

        ImGui::Render();
    }

    printf("DestroyContext()\n");
    ImGui::DestroyContext();

    return EXIT_SUCCESS;
}