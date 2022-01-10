#pragma once

namespace ek {

struct gain_bias_params_t {
    float bias = 0.5f;
    bool gain = true;
};

struct back_params_t {
    float overshoot = 1.70158f;
};

static float graph_biasf(void* data, int idx) {
    auto* params = (gain_bias_params_t*) data;
    const float t = (float) idx / 100.0f;
    return params->gain ? gain_f32(t, params->bias) : bias_f32(t, params->bias);
}

static float graph_back(void* data, int idx) {
    auto* params = (back_params_t*) data;
    const float t = (float) idx / 100.0f;
    return ease_back(t, params->overshoot);
}

void plog_linear() {
    static float x_data[100 + 1];
    static float y_data[100 + 1];
    for (int i = 0; i <= 100; ++i) {
        const float t = (float) i / 100.0f;
        x_data[i] = t;
        y_data[i] = t;
        ImPlot::PlotLine("linear", x_data, y_data, 100 + 1);
    }
}

void drawTestsWindow() {
    {
        static gain_bias_params_t gain_bias{};
        ImGui::DragFloat("Bias", &gain_bias.bias, 0.01f, 0.01f, 0.99f);
        ImGui::Checkbox("Gain", &gain_bias.gain);
        static back_params_t back_params{};
        ImGui::DragFloat("Back Overshoot", &back_params.overshoot, 0.01f, 0.0f, 10.0f);
        if (ImGui::Button("Reset##back_overshoot")) {
            back_params = {};
        }
        if (ImPlot::BeginPlot("Gain Bias", ImVec2(300, 300))) {
            ImPlot::SetupAxes("x", "f(x)");
            plog_linear();
            {
                float x_data[100 + 1];
                float y_data[100 + 1];
                for (int i = 0; i <= 100; ++i) {
                    const float t = (float) i / 100.0f;
                    x_data[i] = t;
                    y_data[i] = gain_bias.gain ? gain_f32(t, gain_bias.bias) : bias_f32(t, gain_bias.bias);
                }
                ImPlot::PlotLine("bias", x_data, y_data, 100 + 1);
            }
            {
                float x_data[100 + 1];
                float y_data[100 + 1];
                for (int i = 0; i <= 100; ++i) {
                    const float t = (float) i / 100.0f;
                    x_data[i] = t;
                    y_data[i] = ease_back(t, back_params.overshoot);
                }
                ImPlot::PlotLine("back", x_data, y_data, 100 + 1);
            }
            ImPlot::EndPlot();
        }
    }


//    if (ImPlot::BeginPlot("Back Overshoot", ImVec2(300, 300))) {
//        ImPlot::SetupAxes("x", "f(x)");
////        plog_linear();
//
//        ImPlot::EndPlot();
//    }
}

void showTestWindow(bool* opened) {
    if (ImGui::Begin("Tests Window###TestWindow", opened)) {
        drawTestsWindow();
    }
    ImGui::End();
}

}