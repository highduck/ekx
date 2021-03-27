#include <ek/ext/analytics/analytics.hpp>
#include <ek/util/logger.hpp>

namespace ek::analytics {

void init() {}

void screen(const char* name) {
    assert(name);
    EK_TRACE("[analytics] screen: %s", name);
}

void event(const char* action, const char* target) {
    EK_TRACE("[analytics] action: %s, target: %s", action, target);
}

}