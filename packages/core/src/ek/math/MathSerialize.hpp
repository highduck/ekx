#pragma once

#include <ek/serialize/serialize.hpp>

#include <ek/math/ColorTransform.hpp>
#include <ek/math/Color32.hpp>
#include <ek/math/MatrixDecl.hpp>
#include <ek/math/Vec.hpp>
#include <ek/math/Rect.hpp>

namespace ek {

template<unsigned N, unsigned M, typename T>
struct declared_as_pod_type<Matrix<N, M, T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<Rect<N, T>> : public std::true_type {
};

template<typename T>
struct declared_as_pod_type<ColorTransform<T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<Vec<N, T>> : public std::true_type {
};

template<>
struct declared_as_pod_type<argb32_t> : public std::true_type {
};

template<>
struct declared_as_pod_type<abgr32_t> : public std::true_type {
};

}