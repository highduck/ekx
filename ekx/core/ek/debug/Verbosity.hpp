#pragma once

namespace ek {

enum class Verbosity : unsigned char {
    None = 0,
    Trace = 1,
    Debug = 2,
    Info = 4,
    Warning = 8,
    Error = 16,
    All = 0xFF
};

inline Verbosity operator&(Verbosity a, Verbosity b) {
    return static_cast<Verbosity>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

inline Verbosity operator&(Verbosity a, unsigned b) {
    return static_cast<Verbosity>(static_cast<unsigned>(a) & b);
}

inline bool operator!(Verbosity a) {
    return a == Verbosity::None;
}

}