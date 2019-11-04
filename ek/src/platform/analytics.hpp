#pragma once

namespace ek::analytics {

void init();

void screen(const char* name);

void event(const char* action, const char* target);

}