#include <plist/plist.hpp>
#include <gtest/gtest.h>

using namespace ek;
using namespace std;
using namespace ek::plist;

template<typename A, typename B>
inline void ASSERT_ARRAY_EQUAL(const A& a, const B& b, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        ASSERT_EQ(a[i], b[i]);
    }
}

template<typename A, typename B>
inline void ASSERT_ARRAY_EQUAL(const A& a, const B& b) {
    ASSERT_EQ(a.size(), b.size());
    ASSERT_ARRAY_EQUAL(a, b, a.size());
}

static void create_message(map<string, any>& dict) {
    date_type date;
    date.set_time_from_apple_epoch(338610664);

    dict["testDate"] = date;
    dict["testInt"] = int(-3455);
    dict["testInt32"] = int32_t(-3455);
    dict["testInt64"] = int64_t(-3455);
    dict["testShort"] = short(3455);
    dict["testLong"] = long(-3455);
    dict["testDouble"] = 1.34223;
    dict["testFloat"] = 1.34223f;
    dict["testBoolTrue"] = true;
    dict["testBoolFalse"] = false;
    std::ifstream stream("test-data/testImage.png", std::ios::binary);
    if (!stream) {
        throw std::runtime_error("Can't open file: test-data/testImage.png");
    }

    int start = stream.tellg();
    stream.seekg(0, std::ifstream::end);
    int size = ((int)stream.tellg()) - start;
    std::vector<char> actual_data(size);
    if (size > 0) {
        stream.seekg(0, std::ifstream::beg);
        stream.read((char*)&actual_data[0], size);
    } else {
        throw std::runtime_error("Can't read zero length data");
    }

    dict["testImage"] = actual_data;
    vector<any> array(2);
    array[0] = 34;
    array[1] = string("string item in array");
    dict["testArray"] = array;
    dict["testString"] = string("hello there");
    dict["testEmptyString"] = string("");

    map<string, any> inner_dict;
    inner_dict["test string"] = string("inner dict item");
    dict["testDict"] = inner_dict;

    inner_dict.clear();
    array.resize(256);

    for (int i = 0; i < 256; ++i) {
        stringstream ss;
        if (i < 10) {
            ss << "00";
        }
        if ((i >= 10) && (i < 100)) {
            ss << "0";
        }
        ss << i;
        array[i] = i;
        inner_dict[ss.str()] = i;
    }
    dict["testArrayLarge"] = array;
    dict["testDictLarge"] = inner_dict;
}

static void check_dictionary(const map<string, any>& dict) {
    string actual_string = "hello there";
    string actual_empty_string = "";
    double actual_double = 1.34223;
    int actual_int = -3455;

    // checking byte array
    std::ifstream stream("test-data/testImage.png", std::ios::binary);
    if (!stream) {
        throw std::runtime_error("Can't open file: test-data/testImage.png");
    }

    int start = stream.tellg();
    stream.seekg(0, std::ifstream::end);
    int size = ((int)stream.tellg()) - start;
    std::vector<char> actual_data(size);
    if (size > 0) {
        stream.seekg(0, std::ifstream::beg);
        stream.read((char*)&actual_data[0], size);
    } else {
        throw std::runtime_error("Can't read zero length data");
    }

    const auto& plist_data = any_cast<const vector<char>&>(dict.find("testImage")->second);

    ASSERT_EQ(actual_data.size(), plist_data.size());
    ASSERT_ARRAY_EQUAL(actual_data.data(), plist_data.data(), actual_data.size());
    ASSERT_FLOAT_EQ(actual_double, any_cast<const double&>(dict.find("testDouble")->second));
    ASSERT_FLOAT_EQ(actual_double, any_cast<const double&>(dict.find("testFloat")->second));
    ASSERT_EQ(actual_int, any_cast<const int64_t&>(dict.find("testInt")->second));
    ASSERT_ARRAY_EQUAL(actual_string.c_str(),
                       any_cast<const string&>(dict.find("testString")->second).c_str(),
                       actual_string.size());

    ASSERT_ARRAY_EQUAL(actual_empty_string.c_str(),
                       any_cast<const string&>(dict.find("testEmptyString")->second).c_str(),
                       actual_empty_string.size());

    {
        // checking array
        const auto& plist_array = any_cast<const vector<any>&>(dict.find("testArray")->second);
        int actual_array_item_0 = 34;
        string actual_array_item_1 = "string item in array";
        ASSERT_EQ(actual_array_item_0, any_cast<const int64_t&>(plist_array[0]));
        ASSERT_ARRAY_EQUAL(actual_array_item_1.c_str(), any_cast<const string&>(plist_array[1]).c_str(),
                           actual_array_item_1.size());
    }

    {
        // checking long array (need to do this because there is different logic if
        // the length of the array is >= 15 elements
        const auto& plist_array_large = any_cast<const vector<any>&>(dict.find("testArrayLarge")->second);
        int i = 0;
        for (auto it = plist_array_large.begin(); i < 256; ++it, ++i) {
            ASSERT_EQ(i, any_cast<const int64_t&>(*it));
        }
    }

    {
        // checking long dict (need to do this because there is different logic if the length
        // of the dict is >= 15 elements
        const auto& plistDictLarge = any_cast<const map<string, any>&>(dict.find("testDictLarge")->second);
        int i = 0;
        for (auto it = plistDictLarge.begin(); i < 256; ++it, ++i) {
            ASSERT_EQ(i, any_cast<const int64_t&>(it->second));
        }
    }

    // checking date
    int actual_date = 338610664;
    ASSERT_EQ(actual_date, (int)any_cast<const date_type&>(dict.find("testDate")->second).time_as_apple_epoch());

    // checking booleans
    ASSERT_TRUE(any_cast<const bool&>(dict.find("testBoolTrue")->second));
    ASSERT_FALSE(any_cast<const bool&>(dict.find("testBoolFalse")->second));
}

TEST(plist, READ_XML) {
    map<string, any> dict;
    read_plist("test-data/XMLExample1.plist", dict);
    check_dictionary(dict);
}

TEST(plist, READ_BINARY) {
    map<string, any> dict;
    read_plist("test-data/binaryExample1.plist", dict);
    check_dictionary(dict);
}

TEST(plist, WRITE_BINARY) {
    map<string, any> dict;
    create_message(dict);

    write_plist_binary("test-data/binaryExampleWritten.plist", dict);
    dict.clear();
    read_plist("test-data/binaryExampleWritten.plist", dict);
    check_dictionary(dict);
}

TEST(plist, WRITE_XML) {
    map<string, any> dict;
    create_message(dict);

    write_plist_xml("test-data/xmlExampleWritten.plist", dict);
    dict.clear();
    read_plist("test-data/xmlExampleWritten.plist", dict);
    check_dictionary(dict);
}

TEST(plist, WRITE_BINARY_TO_BYTE_ARRAY) {
    vector<char> data;
    map<string, any> dict;
    create_message(dict);
    write_plist_binary(data, dict);
    map<string, any> dictCheck;
    read_plist(&data[0], data.size(), dictCheck);
    check_dictionary(dictCheck);
}

TEST(plist, WRITE_XML_TO_BYTE_ARRAY) {
    vector<char> data;
    map<string, any> dict;
    create_message(dict);
    write_plist_xml(data, dict);
    map<string, any> dictCheck;
    read_plist(&data[0], data.size(), dictCheck);
    check_dictionary(dictCheck);
}

TEST(plist, DATE) {
    date_type date;

    // check comparisons
    double objectTime = date.time_as_apple_epoch();

    date_type dateGreater(date);
    dateGreater.set_time_from_apple_epoch(objectTime + 1);
    date_type dateLess(date);
    dateLess.set_time_from_apple_epoch(objectTime - 1);

    ASSERT_EQ(1, date_type::compare(dateGreater, dateLess));
    ASSERT_EQ(-1, date_type::compare(dateLess, dateGreater));
    ASSERT_EQ(0, date_type::compare(date, date));

    ASSERT_TRUE(dateGreater > dateLess);
    ASSERT_TRUE(dateLess < dateGreater);
    ASSERT_TRUE(date == date);

    dateGreater.set_time_from_apple_epoch(objectTime + 100);

    time_t seconds = dateGreater.seconds_since_date(date);
    ASSERT_EQ(100, seconds);
}

//TEST(plist, UNICODE)
//{
//    map<string, any> dict;
//    read_plist("test-data/binaryExampleUnicode.plist", dict);
//}