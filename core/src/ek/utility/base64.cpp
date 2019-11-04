#include "base64.hpp"

#include <vector>

namespace ek {

unsigned char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

array_buffer base64_decode(const std::string& str) {
    std::vector<unsigned char> result;

    // preallocate estimated output size
    result.reserve(str.size() / 4 * 3 + 1);

    constexpr size_t dict_size = 256;
    char in_alphabet[dict_size];
    char decoder[dict_size];
    for (int i = (sizeof alphabet) - 1; i >= 0; --i) {
        in_alphabet[alphabet[i]] = 1;
        decoder[alphabet[i]] = char(i);
    }

    constexpr unsigned int mask = 0xFFU;

    int errors = 0;
    auto* uc_str = reinterpret_cast<const unsigned char*>(str.data());

    unsigned int char_count = 0;
    unsigned int bits = 0;
    unsigned char c = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        c = uc_str[i];
        if (c == '=') {
            break;
        }
        // skip all non-alphabet values (chars)
        if (!in_alphabet[c]) {
            continue;
        }
        bits += decoder[c];
        char_count++;
        if (char_count == 4) {
            result.emplace_back(bits >> 16u);
            result.emplace_back((bits >> 8u) & mask);
            result.emplace_back(bits & 0xFFu);
            bits = 0;
            char_count = 0;
        } else {
            bits <<= 6u;
        }
    }

    if (c == '=') {
        switch (char_count) {
            case 0:
                break;
            case 1:
                // "base64Decode: encoding incomplete: at least 2 bits missing")
                ++errors;
                break;
            case 2:
                result.emplace_back(bits >> 10u);
                break;
            case 3:
                result.emplace_back(bits >> 16u);
                result.emplace_back((bits >> 8u) & mask);
                break;
            default:
                // unreachable code
                break;
        }
    }

    return array_buffer{result.data(), result.size()};
}

std::string base64_decode_str(const std::string& str) {
    auto data = base64_decode(str);
    return std::string{
            reinterpret_cast<const char*>(data.data()),
            data.size()
    };
}

std::string base64_encode(const array_buffer& data) {
    std::vector<char> result;

    // preallocate estimated output size
    result.reserve((data.size() + 2) / 3 * 4);

    // constants
    constexpr unsigned int mask = 0x3Fu;
    constexpr unsigned int shift_0 = 0u;
    constexpr unsigned int shift_1 = 6u;
    constexpr unsigned int shift_2 = 12u;
    constexpr unsigned int shift_3 = 18u;

    // counters
    unsigned int char_count = 0;
    unsigned int bits = 0;

    for (size_t i = 0; i < data.size(); ++i) {
        bits |= data[i];

        ++char_count;
        if (char_count == 3) {
            result.emplace_back(alphabet[(bits >> shift_3) & mask]);
            result.emplace_back(alphabet[(bits >> shift_2) & mask]);
            result.emplace_back(alphabet[(bits >> shift_1) & mask]);
            result.emplace_back(alphabet[(bits >> shift_0) & mask]);
            bits = 0;
            char_count = 0;
        } else {
            bits <<= 8u;
        }
    }

    if (char_count) {
        if (char_count == 1) {
            bits <<= 8u;
        }

        result.emplace_back(alphabet[(bits >> shift_3) & mask]);
        result.emplace_back(alphabet[(bits >> shift_2) & mask]);
        if (char_count > 1) {
            result.emplace_back(alphabet[(bits >> shift_1) & mask]);
        } else {
            result.emplace_back('=');
        }
        result.emplace_back('=');
    }

    return std::string{result.data(), result.size()};
}

std::string base64_encode(const std::string& str) {
    return base64_encode(
            array_buffer(
                    reinterpret_cast<const unsigned char*>(str.data()),
                    str.size()
            )
    );
}

}