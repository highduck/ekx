#pragma once

#ifdef EK_UITEST

#include <ek/assert.hpp>
#include <cstdlib>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/Localization.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Node.hpp>
#include <functional>
#include <unordered_map>

namespace ek::uitest {

extern basic_application* _baseApp;
extern std::string lang;
extern int step;

void screenshot(const char* name);
void done();
void fail();
void click(const std::vector<std::string>& path);
void UITest(const char* name, const std::function<void()>& run);
void initialize(basic_application* baseApp);

}

#endif