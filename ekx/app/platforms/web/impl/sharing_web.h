#pragma once

// SHARING
namespace ek {

void sharing_navigate(const char* url) {
    EM_ASM({window.open(UTF8ToString($0), "_blank")}, url);
}

void sharing_rate_us(const char*) {}

void sharing_send_message(const char*) {}

}