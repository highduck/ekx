#include "plist.hpp"
#include "base64.hpp"

#include <sstream>
#include <algorithm>
#include <cstring>
#include <pugixml.hpp>

using namespace std;
using namespace pugi;

namespace ek::plist {

string utf16_to_utf8(const int16_t* wide_chars, size_t size) {
    abort();
    string res;
    for (size_t i = 0; i < size; ++i) {
        res += static_cast<char>(wide_chars[i] & 0xFF);
    }
    return res;
}

struct plist_helper_data final {
    vector<int32_t> offset_table{};
    vector<unsigned char> object_table{};
    int32_t offset_byte_size{};
    int64_t offset_table_offset{};

    int32_t obj_ref_size{};
    int32_t ref_count{};
};

void write_plist_binary(plist_helper_data& d, const any& message);

void write_plist_xml(xml_document& doc, const any& message);

// helper functions
template<typename T>
T* try_get_vector_data(vector<T>& vec) {
    return vec.empty() ? nullptr : vec.data();
}

template<typename T>
const T* try_get_vector_data(const vector<T>& vec) {
    return vec.empty() ? nullptr : vec.data();
}

// xml helper functions
template<typename T>
string string_from_value(const T& value);

template<typename T>
void write_xml_simple_node(xml_node& node, const char* name, const any& obj);

// xml parsing

dictionary_type parse_dictionary(xml_node& node);

array_type parse_array(xml_node& node);

vector<char> base64_decode(const char* data);

void base64_encode(string& data_encoded, const vector<char>& data);

date_type parse_date(xml_node& node);

any parse(xml_node& doc);

// xml writing
void write_xml_array(xml_node& node, const array_type& array);

void write_xml_dictionary(xml_node& node, const dictionary_type& message);

void write_xml_node(xml_node& node, const any& obj);

// binary helper functions

template<typename IntegerType>
IntegerType bytes_to_int(const unsigned char* bytes, bool little_endian);

double bytes_to_double(const unsigned char* bytes, bool little_endian);

vector<unsigned char> double_to_bytes(double val, bool little_endian);

template<typename IntegerType>
vector<unsigned char> int_to_bytes(IntegerType val, bool little_endian);

vector<unsigned char> get_range(const unsigned char* orig_bytes, int64_t index, int64_t size);

vector<unsigned char> get_range(const vector<unsigned char>& orig_bytes, int64_t index, int64_t size);

vector<char> get_range(const char* orig_bytes, int64_t index, int64_t size);

// binary parsing

any parse_binary(const plist_helper_data& d, int obj_ref);

dictionary_type parse_binary_dictionary(const plist_helper_data& d, int obj_ref);

array_type parse_binary_array(const plist_helper_data& d, int obj_ref);

vector<int32_t> get_refs_for_containers(const plist_helper_data& d, int obj_ref);

int64_t parse_binary_int(const plist_helper_data& d, int header_position, int& int_bytes_count);

double parse_binary_real(const plist_helper_data& d, int header_position);

date_type parse_binary_date(const plist_helper_data& d, int header_position);

bool parse_binary_bool(const plist_helper_data& d, int header_position);

string parse_binary_string(const plist_helper_data& d, int obj_ref);

string parse_binary_unicode(const plist_helper_data& d, int header_position);

data_type parse_binary_byte_array(const plist_helper_data& d, int header_position);

vector<unsigned char> regulate_null_bytes(const vector<unsigned char>& orig_bytes, unsigned int min_bytes_count);

void parse_trailer(plist_helper_data& d, const vector<unsigned char>& trailer);

void parse_offset_table(plist_helper_data& d, const vector<unsigned char>& offset_table_bytes);

int32_t get_count(const plist_helper_data& d, int byte_position, unsigned char header_byte, int& start_offset);

// binary writing

int count_any(const any& object);

int count_dictionary(const dictionary_type& dictionary);

int count_array(const array_type& array);

vector<unsigned char> write_binary_dictionary(plist_helper_data& d, const dictionary_type& dictionary);

vector<unsigned char> write_binary_array(plist_helper_data& d, const array_type& array);

vector<unsigned char> write_binary_byte_array(plist_helper_data& d, const data_type& byte_array);

vector<unsigned char> write_binary_integer(plist_helper_data& d, int64_t value, bool write);

vector<unsigned char> write_binary_bool(plist_helper_data& d, bool value);

vector<unsigned char> write_binary_date(plist_helper_data& d, const date_type& date);

vector<unsigned char> write_binary_double(plist_helper_data& d, double value);

vector<unsigned char> write_binary(plist_helper_data& d, const any& obj);

vector<unsigned char> write_binary_string(plist_helper_data& d, const string& value, bool head);

inline bool host_little_endian() {
    union {
        uint32_t x;
        uint8_t c[4];
    } u{};
    u.x = 0xAB0000CDu;
    return u.c[0] == 0xCDu;
}

template<typename T>
void write_xml_simple_node(xml_node& node, const char* name, const any& obj) {
    xml_node new_node;
    new_node = node.append_child(name);
    new_node.append_child(pugi::node_pcdata).set_value(string_from_value(any_cast<const T&>(obj)).c_str());
}

void write_xml_node(xml_node& node, const any& obj) {
    const auto& obj_type = obj.type();

    if (obj_type == typeid(int32_t))
        write_xml_simple_node<int32_t>(node, "integer", obj);
    else if (obj_type == typeid(int64_t))
        write_xml_simple_node<int64_t>(node, "integer", obj);
    else if (obj_type == typeid(long))
        write_xml_simple_node<long>(node, "integer", obj);
    else if (obj_type == typeid(short))
        write_xml_simple_node<short>(node, "integer", obj);
    else if (obj_type == typeid(dictionary_type))
        write_xml_dictionary(node, any_cast<const dictionary_type&>(obj));
    else if (obj_type == typeid(string_type))
        write_xml_simple_node<string_type>(node, "string", obj);
    else if (obj_type == typeid(const char*))
        write_xml_simple_node<const char*>(node, "string", obj);
    else if (obj_type == typeid(array_type))
        write_xml_array(node, any_cast<const array_type&>(obj));
    else if (obj_type == typeid(data_type)) {
        string data_encoded;
        base64_encode(data_encoded, any_cast<const data_type&>(obj));
        write_xml_simple_node<string>(node, "data", data_encoded);
    } else if (obj_type == typeid(double))
        write_xml_simple_node<double>(node, "real", obj);
    else if (obj_type == typeid(float))
        write_xml_simple_node<float>(node, "real", obj);
    else if (obj_type == typeid(date_type))
        write_xml_simple_node<string>(node, "date", any_cast<const date_type&>(obj).time_as_xml_convention());
    else if (obj_type == typeid(bool)) {
        bool value = any_cast<const bool&>(obj);
        node.append_child(value ? "true" : "false");
    } else
        throw runtime_error((string("Plist Error: Can't serialize type ") + obj_type.name()).c_str());
}

void write_xml_array(xml_node& node, const array_type& array) {
    xml_node new_node = node.append_child("array");
    for (const auto& it : array) {
        write_xml_node(new_node, it);
    }
}

void write_xml_dictionary(xml_node& node, const dictionary_type& message) {
    xml_node new_node = node.append_child("dict");
    for (const auto& it : message) {
        xml_node key_node = new_node.append_child("key");
        key_node.append_child(pugi::node_pcdata).set_value(it.first.c_str());
        write_xml_node(new_node, it.second);
    }
}

void write_plist_xml(xml_document& doc, const any& message) {
    // declaration node
    xml_node decl_node = doc.append_child(pugi::node_declaration);
    decl_node.append_attribute("version") = "1.0";
    decl_node.append_attribute("encoding") = "UTF-8";

    // doctype node
    doc.append_child(pugi::node_doctype)
            .set_value(
                    R"(plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd")"
            );

    // root node
    xml_node plist_node = doc.append_child("plist");
    plist_node.append_attribute("version") = "1.0";

    write_xml_node(plist_node, message);
}

void write_plist_binary(plist_helper_data& d, const any& message) {
    //int total_refs =  count_dictionary(message);
    int total_refs = count_any(message) - 1;
    d.ref_count = total_refs;

    d.obj_ref_size = regulate_null_bytes(int_to_bytes<int32_t>(d.ref_count, host_little_endian()), 1).size();

    //write_binary_dictionary(d, message);
    write_binary(d, message);
    write_binary_string(d, "bplist00", false);
    d.offset_table_offset = (int64_t) d.object_table.size();
    d.offset_table.push_back(d.object_table.size() - 8);
    d.offset_byte_size = regulate_null_bytes(
            int_to_bytes<int>(d.offset_table.back(), host_little_endian()),
            1
    ).size();

    vector<unsigned char> offsetBytes;

    reverse(d.offset_table.begin(), d.offset_table.end());

    for (size_t i = 0; i < d.offset_table.size(); ++i) {
        d.offset_table[i] = d.object_table.size() - d.offset_table[i];
        auto buffer = regulate_null_bytes(
                int_to_bytes<int>(d.offset_table[i], host_little_endian()),
                d.offset_byte_size
        );
        //reverse(buffer.begin(), buffer.end());
        offsetBytes.insert(offsetBytes.end(), buffer.rbegin(), buffer.rend());
    }

    d.object_table.insert(d.object_table.end(), offsetBytes.begin(), offsetBytes.end());

    vector<unsigned char> dummy(6, 0);
    d.object_table.insert(d.object_table.end(), dummy.begin(), dummy.end());
    d.object_table.push_back((unsigned char) (d.offset_byte_size));
    d.object_table.push_back((unsigned char) (d.obj_ref_size));
    auto temp = int_to_bytes<int64_t>((int64_t) total_refs + 1, host_little_endian());
    d.object_table.insert(d.object_table.end(), temp.rbegin(), temp.rend());
    temp = int_to_bytes<int64_t>(0, host_little_endian());
    d.object_table.insert(d.object_table.end(), temp.begin(), temp.end());
    temp = int_to_bytes<int64_t>(d.offset_table_offset, host_little_endian());
    d.object_table.insert(d.object_table.end(), temp.rbegin(), temp.rend());
}

void write_plist_binary(vector<char>& plist, const any& message) {
    plist_helper_data d;
    write_plist_binary(d, message);
    plist.resize(d.object_table.size());
    copy((const char*) try_get_vector_data(d.object_table),
         (const char*) try_get_vector_data(d.object_table) + d.object_table.size(),
         plist.begin());
}

void write_plist_binary(std::ostream& stream, const any& message) {
    plist_helper_data d;
    write_plist_binary(d, message);
    stream.write((const char*) try_get_vector_data(d.object_table), d.object_table.size());
}

void write_plist_binary(const char* filename, const any& message) {
    std::ofstream stream(filename, std::ios::binary);
    write_plist_binary(stream, message);
    stream.close();
}

#if defined(_MSC_VER)
void write_plist_binary(const wchar_t* filename, const any& message) {
    std::ofstream stream(filename, std::ios::binary);
    write_plist_binary(stream, message);
    stream.close();
}
#endif

void write_plist_xml(vector<char>& plist, const any& message) {
    stringstream ss;
    write_plist_xml(ss, message);

    std::istreambuf_iterator<char> beg(ss);
    std::istreambuf_iterator<char> end;
    plist.clear();
    plist.insert(plist.begin(), beg, end);
}

void write_plist_xml(std::ostream& stream, const any& message) {
    xml_document doc;
    write_plist_xml(doc, message);
    doc.save(stream);
}

void write_plist_xml(const char* filename, const any& message) {
    std::ofstream stream(filename, std::ios::binary);
    write_plist_xml(stream, message);
    stream.close();
}

#if defined(_MSC_VER)
void write_plist_xml(const wchar_t* filename, const any& message) {
    std::ofstream stream(filename, std::ios::binary);
    write_plist_xml(stream, message);
    stream.close();
}
#endif

int count_any(const any& object) {
    static any dict = dictionary_type();
    static any array = array_type();

    int count = 0;
    if (object.type() == dict.type()) {
        count += count_dictionary(any_cast<dictionary_type>(object)) + 1;
    } else if (object.type() == array.type()) {
        count += count_array(any_cast<array_type>(object)) + 1;
    } else {
        ++count;
    }

    return count;
}

vector<unsigned char> write_binary(plist_helper_data& d, const any& obj) {
    const auto& obj_type = obj.type();

    vector<unsigned char> value;
    if (obj_type == typeid(int32_t))
        value = write_binary_integer(d, any_cast<const int32_t&>(obj), true);
    else if (obj_type == typeid(int64_t))
        value = write_binary_integer(d, any_cast<const int64_t&>(obj), true);
    else if (obj_type == typeid(long))
        value = write_binary_integer(d, any_cast<const long&>(obj), true);
    else if (obj_type == typeid(short))
        value = write_binary_integer(d, any_cast<const short&>(obj), true);
    else if (obj_type == typeid(dictionary_type))
        value = write_binary_dictionary(d, any_cast<const dictionary_type&>(obj));
    else if (obj_type == typeid(string))
        value = write_binary_string(d, any_cast<const string&>(obj), true);
    else if (obj_type == typeid(array_type))
        value = write_binary_array(d, any_cast<const array_type&>(obj));
    else if (obj_type == typeid(data_type))
        value = write_binary_byte_array(d, any_cast<const data_type&>(obj));
    else if (obj_type == typeid(double))
        value = write_binary_double(d, any_cast<const double&>(obj));
    else if (obj_type == typeid(float))
        value = write_binary_double(d, any_cast<const float&>(obj));
    else if (obj_type == typeid(date_type))
        value = write_binary_date(d, any_cast<const date_type&>(obj));
    else if (obj_type == typeid(bool))
        value = write_binary_bool(d, any_cast<const bool&>(obj));
    else
        throw runtime_error(string("Plist Error: Can't serialize type ") + obj_type.name());

    return value;
}

static uint32_t nextpow2(uint32_t x) {
    --x;
    x |= x >> 1u;
    x |= x >> 2u;
    x |= x >> 4u;
    x |= x >> 8u;
    x |= x >> 16u;
    return x + 1u;
}

static uint32_t ilog2(uint32_t x) {
    uint32_t r = 0;
    while (x >>= 1u) {
        ++r;
    }
    return r;
}

vector<unsigned char> write_binary_byte_array(plist_helper_data& d, const data_type& byte_array) {
    vector<unsigned char> header;
    if (byte_array.size() < 15) {
        header.push_back(0x40u | ((unsigned char) byte_array.size()));
    } else {
        header.push_back(0x40u | 0x0Fu);
        vector<unsigned char> theSize = write_binary_integer(d, byte_array.size(), false);
        header.insert(header.end(), theSize.begin(), theSize.end());
    }

    vector<unsigned char> buffer(header);
    buffer.insert(buffer.end(),
                  (unsigned char*) try_get_vector_data(byte_array),
                  (unsigned char*) try_get_vector_data(byte_array) + byte_array.size());
    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());

    return buffer;
}

vector<unsigned char> write_binary_array(plist_helper_data& d, const array_type& array) {
    vector<int32_t> refs;
    for (auto it = array.rbegin(); it != array.rend(); ++it) {
        write_binary(d, *it);
        d.offset_table.push_back(d.object_table.size());
        refs.push_back(d.ref_count);
        d.ref_count--;
    }

    vector<unsigned char> header;
    if (array.size() < 15) {
        header.push_back(0xA0u | ((unsigned char) array.size()));
    } else {
        header.push_back(0xA0u | 0x0Fu);
        auto the_size = write_binary_integer(d, array.size(), false);
        header.insert(header.end(), the_size.begin(), the_size.end());
    }

    // try to do this more efficiently.  Not good to insert at the beginning of buffer
    vector<unsigned char> buffer;
    for (auto ref : refs) {
        auto ref_buffer = regulate_null_bytes(
                int_to_bytes<int32_t>(ref, host_little_endian()),
                d.obj_ref_size);
        //reverse(ref_buffer.begin(), ref_buffer.end());
        buffer.insert(buffer.begin(), ref_buffer.rbegin(), ref_buffer.rend());
    }

    buffer.insert(buffer.begin(), header.begin(), header.end());

    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());

    return buffer;
}

vector<unsigned char> write_binary_dictionary(plist_helper_data& d, const dictionary_type& dictionary) {
    vector<int32_t> refs;
    for (auto it = dictionary.rbegin(); it != dictionary.rend(); ++it) {
        write_binary(d, it->second);
        d.offset_table.push_back(d.object_table.size());
        refs.push_back(d.ref_count);
        d.ref_count--;
    }

    for (auto it = dictionary.rbegin(); it != dictionary.rend(); ++it) {
        write_binary(d, it->first);
        d.offset_table.push_back(d.object_table.size());
        refs.push_back(d.ref_count);
        d.ref_count--;
    }

    vector<unsigned char> header;
    if (dictionary.size() < 15) {
        header.push_back(0xD0u | ((unsigned char) dictionary.size()));
    } else {
        header.push_back(0xD0u | 0xFu);
        auto the_size = write_binary_integer(d, dictionary.size(), false);
        header.insert(header.end(), the_size.begin(), the_size.end());
    }

    // try to do this more efficiently.  Not good to insert at the beginning of buffer.
    vector<unsigned char> buffer;
    for (auto ref : refs) {
        auto ref_buffer = regulate_null_bytes(
                int_to_bytes<int32_t>(ref, host_little_endian()),
                d.obj_ref_size
        );
//		reverse(ref_buffer.begin(), ref_buffer.end());
        buffer.insert(buffer.begin(), ref_buffer.rbegin(), ref_buffer.rend());
    }

    buffer.insert(buffer.begin(), header.begin(), header.end());

    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());

    return buffer;
}

vector<unsigned char> write_binary_double(plist_helper_data& d, double value) {
    auto buffer = regulate_null_bytes(double_to_bytes(value, host_little_endian()), 4);
    buffer.resize(nextpow2(buffer.size()), 0);

    unsigned char header = 0x20u | ilog2(buffer.size());
    buffer.push_back(header);
    reverse(buffer.begin(), buffer.end());

    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());

    return buffer;
}

vector<unsigned char> write_binary_bool(plist_helper_data& d, bool value) {
    vector<unsigned char> buffer;
    buffer.emplace_back(value ? 0x09 : 0x08);
    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());
    return buffer;
}

vector<unsigned char> write_binary_date(plist_helper_data& d, const date_type& date) {
    vector<unsigned char> buffer;
    // need to serialize as Apple epoch.
    double mac_time = date.time_as_apple_epoch();

    buffer = double_to_bytes(mac_time, false);
    buffer.insert(buffer.begin(), 0x33);

    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());
    return buffer;
}

vector<unsigned char> write_binary_integer(plist_helper_data& d, int64_t value, bool write) {
    // The integer is initially forced to be 64 bit because it must be serialized
    // as 8 bytes if it is negative.   If it is not negative, the
    // regulate_null_bytes step will reduce the representation down to the min
    // power base 2 bytes needed to store it.

    auto buffer = int_to_bytes<int64_t>(value, host_little_endian());
    buffer = regulate_null_bytes(buffer, 1);
    buffer.resize(nextpow2(buffer.size()), 0);

    const unsigned char header = 0x10u | ilog2(buffer.size());
    buffer.push_back(header);
    reverse(buffer.begin(), buffer.end());

    if (write) {
        d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());
    }

    return buffer;
}

vector<unsigned char> write_binary_string(plist_helper_data& d, const string& value, bool head) {
    vector<unsigned char> buffer;
    buffer.reserve(value.size());
    for (auto c : value) {
        buffer.emplace_back(c);
    }

    if (head) {
        vector<unsigned char> header;
        if (value.length() < 15) {
            header.push_back(0x50u | ((unsigned char) value.length()));
        } else {
            header.push_back(0x50u | 0x0Fu);
            auto the_size = write_binary_integer(d, buffer.size(), false);
            header.insert(header.end(), the_size.begin(), the_size.end());
        }
        buffer.insert(buffer.begin(), header.begin(), header.end());
    }

    d.object_table.insert(d.object_table.begin(), buffer.begin(), buffer.end());

    return buffer;
}

int count_dictionary(const dictionary_type& dictionary) {
    int count = 0;
    for (const auto& it : dictionary) {
        ++count;
        count += count_any(it.second);
    }
    return count;
}

int count_array(const array_type& array) {
    int count = 0;
    for (const auto& it : array) {
        count += count_any(it);
    }
    return count;
}

void read_plist(std::istream& stream, any& message) {
    int start = stream.tellg();
    stream.seekg(0, std::ifstream::end);
    int size = ((int) stream.tellg()) - start;
    if (size > 0) {
        stream.seekg(0, std::ifstream::beg);
        vector<char> buffer(size);
        stream.read((char*) &buffer[0], size);

        read_plist(&buffer[0], size, message);
    } else {
        throw runtime_error("Can't read zero length data");
    }
}

void read_plist(const char* byte_array_temp, int64_t size, any& message) {
    const auto* byte_array = reinterpret_cast<const unsigned char*>(byte_array_temp);
    if (!byte_array || (size == 0)) {
        throw runtime_error("Plist: Empty plist data");
    }

    // infer plist type from header.  If it has the bplist00 header as first 8
    // bytes, then it's a binary plist.  Otherwise, assume it's XML
    string magic_header((const char*) byte_array, 8);
    if (magic_header == "bplist00") {
        plist_helper_data d;
        parse_trailer(d, get_range(byte_array, size - 32, 32));

        d.object_table = get_range(byte_array, 0, d.offset_table_offset);
        auto offsetTableBytes = get_range(byte_array, d.offset_table_offset,
                                          size - d.offset_table_offset - 32);

        parse_offset_table(d, offsetTableBytes);
        message = parse_binary(d, 0);
    } else {
        xml_document doc;
        auto result = doc.load_buffer(byte_array, (size_t) size);
        if (!result) {
            throw runtime_error(string("Plist: XML parsed with error ") + result.description());
        }

        xml_node rootNode = doc.child("plist").first_child();
        message = parse(rootNode);
    }
}

dictionary_type parse_dictionary(xml_node& node) {
    dictionary_type dict;
    for (auto it = node.begin(); it != node.end(); ++it) {
        if (string("key") != it->name()) {
            throw runtime_error("Plist: XML dictionary key expected but not found");
        }

        string key(it->first_child().value());
        ++it;

        if (it == node.end()) {
            throw runtime_error("Plist: XML dictionary value expected for key " + key + "but not found");
        } else if (string("key") == it->name()) {
            throw runtime_error("Plist: XML dictionary value expected for key " + key + "but found another key node");
        }

        dict[key] = parse(*it);
    }

    return dict;
}

array_type parse_array(xml_node& node) {
    array_type array;
    for (auto& it : node) {
        array.push_back(parse(it));
    }
    return array;
}

date_type parse_date(xml_node& node) {
    date_type date;
    date.set_time_from_xml_convention(node.first_child().value());
    return date;
}

vector<char> base64_decode(const char* encodedData) {
    vector<char> data;
    insert_iterator<vector<char>> ii(data, data.begin());
    base64<char> b64;
    int state = 0;
    b64.get(encodedData, encodedData + strlen(encodedData), ii, state);

    return data;
}

void base64_encode(string& data_encoded, const vector<char>& data) {
    data_encoded.clear();
    insert_iterator<string> ii(data_encoded, data_encoded.begin());
    base64<char> b64;
    b64.put(data.begin(), data.end(), ii, base64<>::lf());
}

any parse(xml_node& node) {
    string node_name = node.name();

    any result;
    if ("dict" == node_name) {
        result = parse_dictionary(node);
    } else if ("array" == node_name) {
        result = parse_array(node);
    } else if ("string" == node_name) {
        result = string(node.first_child().value());
    } else if ("integer" == node_name) {
        result = (int64_t) strtoll(node.first_child().value(), nullptr, 10);
    } else if ("real" == node_name) {
        result = strtod(node.first_child().value(), nullptr);
    } else if ("false" == node_name) {
        result = bool(false);
    } else if ("true" == node_name) {
        result = bool(true);
    } else if ("data" == node_name) {
        result = base64_decode(node.first_child().value());
    } else if ("date" == node_name) {
        result = parse_date(node);
    } else {
        throw runtime_error("Plist: XML unknown node type " + node_name);
    }

    return result;
}

void parse_offset_table(plist_helper_data& d, const vector<unsigned char>& offset_table_bytes) {
    for (size_t i = 0; i < offset_table_bytes.size(); i += d.offset_byte_size) {
        auto temp = get_range(offset_table_bytes, i, d.offset_byte_size);
        std::reverse(temp.begin(), temp.end());
        d.offset_table.push_back(
                bytes_to_int<int32_t>(
                        try_get_vector_data(
                                regulate_null_bytes(temp, 4)
                        ), host_little_endian()));
    }
}

void parse_trailer(plist_helper_data& d, const vector<unsigned char>& trailer) {
    d.offset_byte_size = bytes_to_int<int32_t>(
            try_get_vector_data(
                    regulate_null_bytes(get_range(trailer, 6, 1), 4)
            ), host_little_endian());

    d.obj_ref_size = bytes_to_int<int32_t>(
            try_get_vector_data(
                    regulate_null_bytes(
                            get_range(trailer, 7, 1), 4)
            ), host_little_endian());

    auto ref_count_bytes = get_range(trailer, 12, 4);
//	std::reverse(ref_count_bytes.begin(), ref_count_bytes.end());
    d.ref_count = bytes_to_int<int32_t>(try_get_vector_data(ref_count_bytes), false);

    auto offset_table_offset_bytes = get_range(trailer, 24, 8);
//	std::reverse(offset_table_offset_bytes.begin(), offset_table_offset_bytes.end());
    d.offset_table_offset = bytes_to_int<int64_t>(
            try_get_vector_data(offset_table_offset_bytes), false);
}


vector<unsigned char> regulate_null_bytes(const vector<unsigned char>& orig_bytes, unsigned int min_bytes_count) {
    vector<unsigned char> bytes(orig_bytes);
    while ((bytes.back() == 0) && (bytes.size() > min_bytes_count)) {
        bytes.pop_back();
    }
    while (bytes.size() < min_bytes_count) {
        bytes.emplace_back(0);
    }
    return bytes;
}

any parse_binary(const plist_helper_data& d, int obj_ref) {
    const unsigned char header = d.object_table[d.offset_table[obj_ref]];
    switch (header & 0xF0u) {
        case 0x00: {
            return parse_binary_bool(d, d.offset_table[obj_ref]);
        }
        case 0x10: {
            int int_bytes_count;
            return parse_binary_int(d, d.offset_table[obj_ref], int_bytes_count);
        }
        case 0x20: {
            return parse_binary_real(d, d.offset_table[obj_ref]);
        }
        case 0x30: {
            return parse_binary_date(d, d.offset_table[obj_ref]);
        }
        case 0x40: {
            return parse_binary_byte_array(d, d.offset_table[obj_ref]);
        }
        case 0x50: {
            return parse_binary_string(d, d.offset_table[obj_ref]);
        }
        case 0x60: {
            return parse_binary_unicode(d, d.offset_table[obj_ref]);
        }
        case 0xD0: {
            return parse_binary_dictionary(d, obj_ref);
        }
        case 0xA0: {
            return parse_binary_array(d, obj_ref);
        }
    }
    throw runtime_error("This type is not supported");
}

vector<int32_t> get_refs_for_containers(const plist_helper_data& d, int obj_ref) {
    int32_t ref_count = 0;
    int ref_start_position;
    ref_count = get_count(d, d.offset_table[obj_ref], d.object_table[d.offset_table[obj_ref]], ref_start_position);
    ref_start_position += d.offset_table[obj_ref];

    vector<int32_t> refs;
    int multiplier = 1;
    if ((((unsigned char) d.object_table[d.offset_table[obj_ref]]) & 0xF0u) == 0xD0u) {
        multiplier = 2;
    }
    for (int i = ref_start_position;
         i < ref_start_position + ref_count * multiplier * d.obj_ref_size;
         i += d.obj_ref_size) {

        auto ref_buffer = get_range(d.object_table, i, d.obj_ref_size);
        reverse(ref_buffer.begin(), ref_buffer.end());
        refs.push_back(
                bytes_to_int<int32_t>(
                        try_get_vector_data(
                                regulate_null_bytes(ref_buffer, 4)
                        ), host_little_endian()
                )
        );
    }

    return refs;
}

array_type parse_binary_array(const plist_helper_data& d, int obj_ref) {
    vector<int32_t> refs = get_refs_for_containers(d, obj_ref);
    array_type array;
    for (auto ref : refs) {
        array.push_back(parse_binary(d, ref));
    }
    return array;
}

dictionary_type parse_binary_dictionary(const plist_helper_data& d, int obj_ref) {
    vector<int32_t> refs = get_refs_for_containers(d, obj_ref);
    int32_t ref_count = refs.size() / 2;

    dictionary_type dict;
    for (int i = 0; i < ref_count; i++) {
        any key_any = parse_binary(d, refs[i]);

        try {
            string& key = any_cast<string&>(key_any);
            dict[key] = parse_binary(d, refs[i + ref_count]);
        }
        catch (std::bad_any_cast&) {
            throw runtime_error("Error parsing dictionary.  Key can't be parsed as a string");
        }
    }

    return dict;
}

string parse_binary_string(const plist_helper_data& d, int obj_ref) {
    unsigned char header_byte = d.object_table[obj_ref];
    int char_start_position;
    int32_t char_count = get_count(d, obj_ref, header_byte, char_start_position);
    char_start_position += obj_ref;

    auto character_bytes = get_range(d.object_table, char_start_position, char_count);
    return string((char*) try_get_vector_data(character_bytes), character_bytes.size());
}

string parse_binary_unicode(const plist_helper_data& d, int header_position) {
    unsigned char header_byte = d.object_table[header_position];
    int char_start_position;
    int32_t char_count = get_count(d, header_position, header_byte, char_start_position);
    char_start_position += header_position;

    auto character_bytes = get_range(d.object_table, char_start_position, char_count * 2);
    if (host_little_endian()) {
        if (!character_bytes.empty()) {
            for (size_t i = 0, n = character_bytes.size(); i < n - 1; i += 2) {
                swap(character_bytes[i], character_bytes[i + 1]);
            }
        }
    }

    const auto* u16chars = reinterpret_cast<const int16_t*>(try_get_vector_data(character_bytes));
    size_t u16len = character_bytes.size() / 2;
    string result = utf16_to_utf8(u16chars, u16len);
    return result;
}

int64_t parse_binary_int(const plist_helper_data& d, int header_position, int& int_bytes_count) {
    unsigned char header = d.object_table[header_position];
    int_bytes_count = 1u << (header & 0x0Fu);
    vector<unsigned char> buffer = get_range(d.object_table, header_position + 1, int_bytes_count);
    reverse(buffer.begin(), buffer.end());
    return bytes_to_int<int64_t>(
            try_get_vector_data(
                    regulate_null_bytes(buffer, 8)
            ), host_little_endian()
    );
}

double parse_binary_real(const plist_helper_data& d, int header_position) {
    unsigned char header = d.object_table[header_position];
    int bytes_count = 1u << (header & 0x0Fu);
    auto buffer = get_range(d.object_table, header_position + 1, bytes_count);
    reverse(buffer.begin(), buffer.end());
    return bytes_to_double(
            try_get_vector_data(
                    regulate_null_bytes(buffer, 8)
            ), host_little_endian()
    );
}

bool parse_binary_bool(const plist_helper_data& d, int header_position) {
    unsigned char header = d.object_table[header_position];
    bool value;
    if (header == 0x09u) {
        value = true;
    } else if (header == 0x08u) {
        value = false;
    } else if (header == 0x00u) {
        // null byte, not sure yet what to do with this.  It's in the spec but we
        // have never encountered it.
        throw runtime_error("Plist: null byte encountered, unsure how to parse");
    } else if (header == 0x0Fu) {
        // fill byte, not sure yet what to do with this.  It's in the spec but we
        // have never encountered it.
        throw runtime_error("Plist: fill byte encountered, unsure how to parse");
    } else {
        stringstream ss;
        ss << "Plist: unknown header " << header;
        throw runtime_error(ss.str().c_str());
    }
    return value;
}

date_type parse_binary_date(const plist_helper_data& d, int header_position) {
    // date always an 8 byte float starting after full byte header
    auto buffer = get_range(d.object_table, header_position + 1, 8);
    date_type date;
    // date_type is stored as Apple Epoch and big endian.
    date.set_time_from_apple_epoch(
            bytes_to_double(try_get_vector_data(buffer), false)
    );
    return date;
}

data_type parse_binary_byte_array(const plist_helper_data& d, int header_position) {
    unsigned char header_byte = d.object_table[header_position];
    int byte_start_position;
    int32_t byteCount = get_count(d, header_position, header_byte, byte_start_position);
    byte_start_position += header_position;

    return get_range((const char*) try_get_vector_data(d.object_table), byte_start_position, byteCount);
}

int32_t get_count(const plist_helper_data& d, int byte_position, unsigned char header_byte, int& start_offset) {
    unsigned char headerByteTrail = header_byte & 0x0Fu;
    if (headerByteTrail < 15) {
        start_offset = 1;
        return headerByteTrail;
    } else {
        auto count = (int32_t) parse_binary_int(d, byte_position + 1, start_offset);
        start_offset += 2;
        return count;
    }
}

template<typename T>
string string_from_value(const T& value) {
    stringstream ss;
    ss << value;
    return ss.str();
}

template<typename IntegerType>
IntegerType bytes_to_int(const unsigned char* bytes, bool little_endian) {
    IntegerType result = 0;
    if (little_endian) {
        for (int n = sizeof(result) - 1; n >= 0; --n) {
            result = (result << 8u) + bytes[n];
        }
    } else {
        for (unsigned n = 0; n < sizeof(result); ++n) {
            result = (result << 8u) + bytes[n];
        }
    }
    return result;
}

double bytes_to_double(const unsigned char* bytes, bool little_endian) {
    double result;
    int num_bytes = sizeof(double);
    if (little_endian)
        memcpy(&result, bytes, num_bytes);
    else {
        vector<unsigned char> bytesReverse(num_bytes);
        reverse_copy(bytes, bytes + num_bytes, bytesReverse.begin());
        memcpy(&result, try_get_vector_data(bytesReverse), num_bytes);
    }
    return result;
}

vector<unsigned char> double_to_bytes(double val, bool little_endian) {
    vector<unsigned char> result(sizeof(double));
    memcpy(try_get_vector_data(result), &val, sizeof(double));
    if (!little_endian) {
        reverse(result.begin(), result.end());
    }
    return result;
}

template<typename IntegerType>
vector<unsigned char> int_to_bytes(IntegerType val, bool little_endian) {
    unsigned int num_bytes = sizeof(val);
    vector<unsigned char> bytes(num_bytes);

    for (unsigned n = 0; n < num_bytes; ++n) {
        if (little_endian) {
            bytes[n] = (val >> (8 * n)) & 0xFFu;
        } else {
            bytes[num_bytes - 1 - n] = (val >> (8 * n)) & 0xFFu;
        }
    }

    return bytes;
}

vector<unsigned char> get_range(const unsigned char* orig_bytes, int64_t index, int64_t size) {
    vector<unsigned char> result((vector<unsigned char>::size_type) size);
    copy(orig_bytes + index, orig_bytes + index + size, result.begin());
    return result;
}

vector<char> get_range(const char* orig_bytes, int64_t index, int64_t size) {
    vector<char> result((vector<char>::size_type) size);
    copy(orig_bytes + index, orig_bytes + index + size, result.begin());
    return result;
}

vector<unsigned char> get_range(const vector<unsigned char>& orig_bytes, int64_t index, int64_t size) {
    if ((index + size) > (int64_t) orig_bytes.size()) {
        throw runtime_error("Out of bounds get_range");
    }
    return get_range(try_get_vector_data(orig_bytes), index, size);
}

}