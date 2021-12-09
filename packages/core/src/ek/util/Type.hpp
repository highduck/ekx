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

//#define EK_TYPE_INDEX_T(Bs, Tp, Idx) template<> struct TypeIndex<Tp,Bs>{constexpr static int value = (Idx);}
//#define EK_TYPE_INDEX(Tp, Idx) template<> struct TypeIndex<Tp,void>{constexpr static int value = (Idx);}

//#define EK_TYPE_INDEX_T(Bs, Tp, Idx) template<> struct TypeIndex<Tp,Bs>;
//#define EK_TYPE_INDEX(Tp, Idx) template<> struct TypeIndex<Tp,void>;

#define EK_TYPE_INDEX_T(Bs, Tp, Idx)
#define EK_TYPE_INDEX(Tp, Idx)
