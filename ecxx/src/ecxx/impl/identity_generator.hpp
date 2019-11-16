#pragma once

namespace ecxx {
namespace details {

template<typename Identity>
struct identity_counter {
    static Identity counter;

    inline static Identity next() {
        return counter++;
    }
};

template<typename Identity>
Identity identity_counter<Identity>::counter = Identity(0);

}

// 1. match between TU
// 2. starts from 0 for each Identity type
template<typename T, typename Identity>
struct identity_generator {
    static const Identity value;
};

template<typename T, typename Identity>
const Identity identity_generator<T, Identity>::value = details::identity_counter<Identity>::next();

}