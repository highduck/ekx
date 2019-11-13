#pragma once

#include <scenex/ek/basic_application.h>

#ifdef EK_EDITOR

#include <ek/editor/editor.hpp>

#endif

class PikoApp : public scenex::basic_application {
public:
    PikoApp();

    ~PikoApp() override;

    void initialize() override;

    void preload() override;

protected:
    void update_frame(float dt) override;

    void render_frame() override;

#ifdef EK_EDITOR
    ek::editor_context_t editor_;
#endif
};