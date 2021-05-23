#pragma once

namespace ek {

struct BaseType {
    const char* label = nullptr;
};

template<typename T>
struct Type : public BaseType {
    static Type<T> Data;

    Type() = default;

    explicit Type(const char* l) {
        label = l;
    }
};

template<typename T>
inline Type<T> Type<T>::Data{};

#define EK_DECLARE_TYPE(Tp) template<> inline ek::Type<Tp>  ek::Type<Tp>::Data{#Tp}

template<typename T>
inline Type<T>& getType() {
    return Type<T>::Data;
}

}