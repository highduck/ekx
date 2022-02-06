#ifdef EK_UITEST

#include "uitest.h"

namespace ek::uitest {

basic_application* _baseApp = nullptr;
int step = 0;
std::unordered_map<std::string, std::function<void()>> _tests{};
std::string _testToRun;
std::string screenshotOutputDir;
std::string lang{};

void setSimulatorDisplaySettings(float2 size, float4 insets, bool relative, float2 applicationBaseSize) {
    if (relative) {
        const float scale = 4.0f;
        const float2 aspect = size;
        size.x = applicationBaseSize.x * scale;
        size.y = size.x * aspect.y / aspect.x;
        insets *= applicationBaseSize.x * scale;
    }

    _baseApp->display.simulated = true;
    _baseApp->display.info.size =
    _baseApp->display.info.window = size;
    _baseApp->display.info.dpiScale = 1.0f;
    _baseApp->display.info.insets = insets;
}

void screenshot(const char* name) {
    ++step;
    _baseApp->display.screenshot((screenshotOutputDir + std::to_string(step) + '_' + name + ".png").c_str());
}

void done() {
    exit(0);
}

void fail() {
    exit(1);
}

void click(const std::vector<std::string>& path) {
    auto e = findByPath(_baseApp->root, path);
    if (e.is_alive()) {
        auto* btn = e.tryGet<Button>();
        if (btn != nullptr) {
            btn->clicked();
        }
    }
}

void runUITest() {
    if (!lang.empty()) {
        s_localization.setLanguage(lang);
    }

    if (_testToRun.empty()) {
        // bypass for debugging
        return;
    }
    auto it = _tests.find(_testToRun);
    if (it != _tests.end()) {
        log_info("UI test run: %s", _testToRun.c_str());
        it->second();
        return;
    }
    log_error("UI test %s not found", _testToRun.c_str());
    fail();
}

void UITest(const char* name, const std::function<void()>& run) {
    _tests[name] = run;
}

class UITestStartListener : public GameAppListener {
public:
    ~UITestStartListener() override = default;

    void onStart() override {
        runUITest();
    }
};

void initialize(basic_application* baseApp) {
    _baseApp = baseApp;
    EK_ASSERT(_baseApp != nullptr);

    _testToRun = app::findArgumentValue("--uitest", "");
    lang = app::findArgumentValue("--lang", "");
    {
        float2 size{1, 1};
        float4 insets{0, 0, 0, 0};
        int flags = 0;
        sscanf(
                app::findArgumentValue("--display", "9,16,0,0,0,0,1"),
                "%f,%f,%f,%f,%f,%f,%d",
                size.data(), size.data() + 1,
                insets.data(), insets.data() + 1, insets.data() + 2, insets.data() + 3,
                &flags
        );
        const float2 appSize{480, 640};
        setSimulatorDisplaySettings(size, insets, flags == 1, appSize);
    }
    screenshotOutputDir = app::findArgumentValue("--screenshot-output", "");

    static UITestStartListener uiTestStarter{};
    _baseApp->dispatcher.listeners.push_back(&uiTestStarter);
}

}

#endif // EK_UITEST
