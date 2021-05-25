#include <doctest.h>

#include "Array.hpp"

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

TEST_CASE("Array moveCopy") {
    using namespace ek;

    memory::initialize();
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
    memory::shutdown();
}
