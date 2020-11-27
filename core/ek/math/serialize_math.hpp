#pragma once

#include <ek/serialize/serialize.hpp>

#include <ek/math/color_transform.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/transform.hpp>
#include <ek/math/matrix_decl.hpp>
#include <ek/math/vec.hpp>
#include <ek/math/box.hpp>

namespace ek {

template<typename T>
struct declared_as_pod_type<transform_t<T>> : public std::true_type {
};

template<unsigned N, unsigned M, typename T>
struct declared_as_pod_type<matrix_t<N, M, T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<box_t<N, T>> : public std::true_type {
};

template<typename T>
struct declared_as_pod_type<color_transform_t<T>> : public std::true_type {
};

template<typename T, unsigned N>
struct declared_as_pod_type<vec_t<N, T>> : public std::true_type {
};

template<>
struct declared_as_pod_type<argb32_t> : public std::true_type {
};

template<>
struct declared_as_pod_type<abgr32_t> : public std::true_type {
};

}