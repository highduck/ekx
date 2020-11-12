#pragma once

#include <cstdint>
#include <memory>

namespace ek {

class Profiler {
public:
    Profiler();

    ~Profiler();

    void update();

    void draw();

    void setGameTime(float time);
private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

}

