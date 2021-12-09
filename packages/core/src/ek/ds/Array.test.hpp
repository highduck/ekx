#include <doctest.h>

#include "Array.hpp"

TEST_SUITE_BEGIN("c++ array");

struct NonTrivialStruct {
    int counter = 0;

    NonTrivialStruct() {
        ++counter;
    }

    // issue `crash on Array::push_back with std::string`
    NonTrivialStruct& operator=(const NonTrivialStruct& o) {
        // check if object is alive (valid state)
        // this case brake if data structure use COPY ASSIGNMENT
        // on invalid state object after destructor was called
        EK_ASSERT(counter > 0);
        counter = o.counter;
        return *this;
    }

    ~NonTrivialStruct() {
        --counter;
        EK_ASSERT(counter == 0);
    }
};

TEST_CASE("move and copy") {
    using namespace ek;
    {
        Array<int> a;
        a.push_back(1);
        Array<int> movedA{std::move(a)};
        movedA.push_back(3);

        REQUIRE(movedA[0] == 1);
        REQUIRE(movedA[1] == 3);
    }
    {
        Array<NonTrivialStruct> a;
        a.push_back(NonTrivialStruct());
        a.eraseAt(0u);
        a.push_back(NonTrivialStruct());
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> movedA = std::move(a);
        movedA.push_back(3);

        REQUIRE(movedA[0] == 1);
        REQUIRE(movedA[1] == 3);
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> copyA{a};
        copyA.push_back(3);
        a.push_back(2);

        REQUIRE(copyA[0] == 1);
        REQUIRE(copyA[1] == 3);
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
    }
    {
        Array<int> a;
        a.push_back(1);
        Array<int> copyA = a;
        copyA.push_back(3);
        a.push_back(2);

        REQUIRE(copyA[0] == 1);
        REQUIRE(copyA[1] == 3);
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
    }
}

TEST_CASE("push, erase, insert") {

    ek::Array<int> arr{};

    REQUIRE(arr.size() == 0);
    for (int i = 0; i < 20000; i += 50) {
        for (int j = 0; j < i; ++j) {
            arr.push_back(j);
        }
        arr.reset();
    }

    for (int i = 0; i < 4; ++i) {
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        arr.eraseAt(i);
        arr.reset();
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        arr.push_back(4);
        arr.swapRemove(i);
        arr.reset();
    }

    // TODO: insert
//    for (int i = 0; i < 5; ++i) {
//        arr.push_back(1);
//        arr.push_back(2);
//        arr.push_back(3);
//        arr.push_back(4);
//        arr.insert(i, 5);
//        REQUIRE(arr[i] == 5);
//        if (i < 4) {
//            REQUIRE(arr[4] == 4);
//        }
//        arr.reset();
//    }
}

TEST_SUITE_END();