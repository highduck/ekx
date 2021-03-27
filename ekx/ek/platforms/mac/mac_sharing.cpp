#include <string>

namespace ek {

void sharing_navigate(const char* url) {
    std::string cmd = "open ";
    cmd += url;
    system(cmd.c_str());
}

void sharing_rate_us(const char* app_id) {
    (void) app_id;
    // TODO:
}

void sharing_send_message(const char* text) {
    (void) text;
    // TODO:
}

}