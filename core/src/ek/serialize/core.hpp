#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>

namespace ek {

template<typename T>
struct declared_as_pod_type : public std::false_type {
};

template<typename T>
struct is_readable_stream_type : public std::false_type {
};

// TODO: could be templated variables

template<typename Stream>
constexpr bool is_readable_stream() {
    return is_readable_stream_type<Stream>::value;
}

template<typename Stream>
constexpr bool is_writable_stream() {
    return !is_readable_stream_type<Stream>::value;
}

template<typename T>
constexpr bool is_pod_type() {
    return declared_as_pod_type<T>::value
           || std::is_integral<T>::value
           || std::is_floating_point<T>::value
           || std::is_enum<T>::value;
}

template<typename Stream>
class IO {
public:
    Stream& stream;

    explicit IO(Stream& s)
            : stream{s} {
    }

    template<typename T>
    inline void value(T& value_) {
        if constexpr (is_readable_stream<Stream>()) {
            stream.read(value_);
        } else {
            stream.write(value_);
        }
    }

    inline void span(const void* data, uint32_t size) {
        if constexpr (is_readable_stream<Stream>()) {
            stream.read(const_cast<void*>(data), size);
        } else {
            stream.write(data, size);
        }
    }

    template<typename... Args>
    inline void operator()(Args&& ... args) {
        process_args(std::forward<Args>(args)...);
    }

private:

    template<typename T, typename... Args>
    inline void process_args(T&& first, Args&& ... args) {
        serialize(*this, std::forward<T>(first));
        process_args(std::forward<Args>(args)...);
    }

    inline void process_args() {}
};

struct serializer_access {
    template<typename AbstractIO, typename T>
    inline static void serialize(AbstractIO& io, T& value) {
        value.serialize(io);
    }
};

template<typename Stream, typename T>
inline void serialize(IO<Stream>& io, T& value_) {
    if constexpr(is_pod_type<T>()) {
        io.value(value_);
    } else {
        serializer_access::serialize(io, value_);
    }
}

}