#include "parsing.hpp"

#include <ek/flash/doc/types.hpp>
#include <pugixml.hpp>

#include <string>
#include <vector>

namespace ek::flash {

std::vector<std::string> split(const std::string& str, const char separator) {
    using size_type = std::string::size_type;

    std::vector<std::string> result;
    size_type prevPos = 0;
    size_type pos = 0;

    while ((pos = str.find(separator, pos)) != std::string::npos) {
        std::string substring{str.substr(prevPos, pos - prevPos)};
        result.push_back(substring);
        prevPos = ++pos;
    }

    // Last word
    result.push_back(str.substr(prevPos, pos - prevPos));

    return result;
}

double read_double_hex(const char* str, int len) {
    if (len == 0) {
        return 0.0;
    }

    if (str[0] == '#') {
        char* dot = nullptr;
        uint32_t hex = strtoul(str + 1, &dot, 16);
        hex = hex << 8;
        if (dot && dot[0] == '.') {
            auto n = len - int(dot - str + 1);
            uint32_t postfix = strtoul(dot + 1, nullptr, 16);
            while (n < 2) {
                // each trailing zero is 4-bit
                postfix = postfix << 4;
                ++n;
            }
            hex = hex | postfix;
        }
        return (*reinterpret_cast<int32_t*>(&hex)) / double(1 << 8);
    }

    // default floating point format
    return strtod(str, nullptr);
}

double read_twips(const char* str, int n) {
    return read_double_hex(str, n) / 20.0;
}

bool is_whitespace(char c) {
    return c == '\n' || c == '\r' || c == ' ' || c == '\t';
}

bool is_cmd(char c) {
    return c == '!' || c == '|' || c == '/' || c == '[' || c == ']' || c == 'S';
}

bool is_eos(char c) {
    return c == '\0';
}

int get_value_length(const char* buf) {
    const char* ptr = buf;
    char c = *ptr;
    while (!is_eos(c) && !is_cmd(c) && !is_whitespace(c)) {
        ++ptr;
        c = *ptr;
    }
    return ptr - buf;
}

void parse_edges(const char* data, std::vector<char>& out_commands, std::vector<double>& out_values) {
    if (!data) {
        return;
    }

    char c = *data;
    while (!is_eos(c)) {
        if (is_whitespace(c)) {

        } else if (is_cmd(c)) {
            out_commands.push_back(c);
        } else {
            int l = get_value_length(data);
            out_values.push_back(read_twips(data, l));
            data += l - 1;
        }

        ++data;
        c = *data;
    }
}

edge_t& operator<<(edge_t& r, const xml_node& node) {
    parse_edges(node.attribute("edges").value(), r.commands, r.values);
    r.fill_style_0 = node.attribute("fillStyle0").as_int();
    r.fill_style_1 = node.attribute("fillStyle1").as_int();
    r.stroke_style = node.attribute("strokeStyle").as_int();
    return r;
}

}