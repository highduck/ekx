#pragma once

namespace ek {

template<typename T>
struct TypeName {
    constexpr static const char* value{""};
};

#define EK_DECLARE_TYPE(Tp)  template<> struct TypeName<Tp>{constexpr static const char* value = #Tp;}

template<typename A>
struct Counter {
    inline static int value = 0;
};

template<typename T, typename Tag = void>
struct TypeIndex {
    inline const static int value = Counter<Tag>::value++;
};

}

