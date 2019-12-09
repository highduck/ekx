#pragma once

#include <ek/serialize/serialize.hpp>

#include <ek/math/color_transform.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/math/vec_fwd.hpp>
#include <ek/math/box.hpp>

namespace ek {

template<unsigned N, unsigned M, typename T>
struct declared_as_pod_type<matrix_t<N, M, T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<box_t<T, N>> : public std::true_type {
};

template<typename T>
struct declared_as_pod_type<color_transform_t<T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<vec_t<T, N>> : public std::true_type {
};

template<>
struct declared_as_pod_type<argb32_t> : public std::true_type {
};

template<>
struct declared_as_pod_type<abgr32_t> : public std::true_type {
};

template<>
struct declared_as_pod_type<premultiplied_abgr32_t> : public std::true_type {
};

}