#include <gtest/gtest.h>
#include <ek/util/common_macro.hpp>


using namespace ek;

struct test_type1 {
    test_type1() {
        assert_created_once<test_type1>();
    }
};

struct test_type2 {
    test_type2() {
        assert_created_once<test_type2>();
    }
};

struct test_type3 {
    test_type3() {
        assert_created_once<test_type3>();
    }
};

struct test_type4 {
    test_type4() {
        assert_created_once<test_type4>();
    }
};

void st1() {
    test_type4 t4_repeated{};
}

void st2() {
    assert_single_instance_t<test_type1> t1_repeated{};
}

TEST(common_macro, assert_created_once) {
    {
        test_type1 t1{};
        test_type2 t2{};
        test_type3 t3{};
        test_type4 t4{};
    }

    // asserts works only for DEBUG builds
#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
    ASSERT_DEATH({
                     st1();
                 }, ".*");
#endif
}

TEST(common_macro, assert_single_instance) {
    {
        assert_single_instance_t<test_type1> t1{};
        assert_single_instance_t<test_type2> t2{};
        assert_single_instance_t<test_type3> t3{};
        assert_single_instance_t<test_type4> t4{};
    }
    {
        assert_single_instance_t<test_type1> t1{};
        assert_single_instance_t<test_type2> t2{};
        assert_single_instance_t<test_type3> t3{};
        assert_single_instance_t<test_type4> t4{};

        // asserts works only for DEBUG builds
#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
        ASSERT_DEATH({
                         st2();
                     }, ".*");
#endif
    }
}
