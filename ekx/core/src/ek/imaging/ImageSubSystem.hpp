#pragma once

namespace ek {

class Allocator;

namespace imaging {

extern Allocator& allocator;

void initialize();
void shutdown();

}

}