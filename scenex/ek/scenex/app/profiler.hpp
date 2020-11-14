#pragma once

#include <cstdint>
#include <memory>

namespace ek {

class Profiler {
public:
    Profiler();

    ~Profiler();

    void update(float dt);

    void draw();

    void addTime(const char* name, float time);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

}

