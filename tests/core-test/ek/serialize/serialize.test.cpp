#include <ek/serialize/serialize.hpp>

#include <gtest/gtest.h>
#include <iostream>
#include <array>

using namespace ek;

enum class TestFlagsCpp : uint8_t {
    One = 1,
    Two = 2,
    Three = 88
};

struct GlyphModel {
    std::vector<uint32_t> codes;
    std::array<int, 4> box;
    int advanceWidth;
    std::string sprite;

    template<typename S>
    void serialize(IO<S>& io) {
        io(codes, box, advanceWidth, sprite);
    }
};

struct FontModel {
    uint16_t unitsPerEm;
    std::vector<GlyphModel> glyphs;
    std::vector<uint16_t> sizes;

    template<typename S>
    void serialize(IO<S>& io) {
        io(unitsPerEm, sizes, glyphs);
    }
};

class Bar {
public:
    /* 4 */ float f1 = 2.0f;
    /* 4 */ int i4 = 4;
    /* 8 */ double d3 = 3;
    /* 1 */ bool flag = true;

    template<typename Stream>
    void serialize(IO<Stream>& io) {
        io(f1, i4, d3, flag);
    }
};

template<typename T>
class Vector2 {
public:
    union {
        struct {
            T x;
            T y;
        };
        T values[2];
    };

    Vector2() = default;

    Vector2(T px, T py)
            : x{px}, y{py} {

    }
};

template<typename T>
struct declared_as_pod_type<Vector2<T>> : public std::true_type {
};

TEST(Serialization, Basic) {

    Bar bar{};

    output_memory_stream writeStream{200};
    IO writer{writeStream};

    std::string emptyString;
    auto ff = TestFlagsCpp::Three;
    writer(bar, ff, emptyString);

//    writer(bar, TestFlagsCpp::Three);

    ASSERT_EQ(writeStream.size(), 17 + 1 + 4);

    input_memory_stream read1{writeStream.data(), writeStream.size()};
    IO io{read1};

    Bar barLoaded{};
    TestFlagsCpp f{};
    std::string str{"hi"};
    io(barLoaded, f, str);
    ASSERT_FLOAT_EQ(barLoaded.f1, 2.0f);
    ASSERT_FLOAT_EQ(barLoaded.d3, 3);
    ASSERT_EQ(barLoaded.i4, 4);
    ASSERT_EQ(barLoaded.flag, true);
    ASSERT_EQ(f, TestFlagsCpp::Three);
    ASSERT_TRUE(str.empty());

    input_memory_stream read2{writeStream.data(), writeStream.size()};
    IO<input_memory_stream> io2{read2};

    Bar barCorrupted{};
    io2(barCorrupted);
    ASSERT_FLOAT_EQ(barLoaded.f1, 2.0f);
    ASSERT_FLOAT_EQ(barLoaded.d3, 3);
    ASSERT_EQ(barLoaded.i4, 4);
}

TEST(Serialization, Basic2) {

    GlyphModel glyph{
            {1, 2, 3},
            {0, 1, 2, 3},
            -3,
            "text"
    };

    output_memory_stream writeStream{200};
    IO out{writeStream};
    out(glyph);

    GlyphModel s;

    input_memory_stream readStream{writeStream.data(), writeStream.size()};
    IO in{readStream};
    in(s);

    ASSERT_EQ(s.advanceWidth, -3);
    ASSERT_EQ(s.sprite, "text");
    ASSERT_EQ(s.box[3], 3);
    ASSERT_EQ(s.codes.size(), 3);
    ASSERT_EQ(s.codes[0], 1);

}

TEST(Serialization, Nesting) {

    GlyphModel glyph1{
            {1, 2, 3},
            {0, 1, 2, 3},
            -3,
            "1"
    };

    GlyphModel glyph2{
            {4, 5, 6},
            {2, 20, 200, 2000},
            -3,
            "2"
    };

    FontModel font{
            2048u,
            {
                    glyph1,
                    glyph2
            },
            {8, 16, 24, 32}
    };

    output_memory_stream outs{200};
    IO out{outs};
    out(font);

    FontModel s;

    input_memory_stream readStream{outs.data(), outs.size()};
    IO in{readStream};
    in(s);

    ASSERT_EQ(s.glyphs.size(), 2);
    ASSERT_EQ(s.glyphs[0].sprite, glyph1.sprite);
    ASSERT_EQ(s.glyphs[0].box, glyph1.box);
    ASSERT_EQ(s.glyphs[0].codes, glyph1.codes);
    ASSERT_EQ(s.glyphs[1].sprite, glyph2.sprite);
    ASSERT_EQ(s.glyphs[1].box, glyph2.box);
    ASSERT_EQ(s.glyphs[1].codes, glyph2.codes);
    ASSERT_EQ(s.unitsPerEm, 2048);
    ASSERT_EQ(s.sizes[3], 32);
}

TEST(Serialization, Enum) {
    output_memory_stream outs{200};
    IO writer{outs};

    TestFlagsCpp a{TestFlagsCpp::One};
    TestFlagsCpp b{TestFlagsCpp::Two};
    TestFlagsCpp c{TestFlagsCpp::Three};
    std::vector<TestFlagsCpp> d{TestFlagsCpp::One, TestFlagsCpp::Two, TestFlagsCpp::Three};
    writer(a, b, c, d);

    ASSERT_EQ(outs.size(), 3 + 4 + 3);

    input_memory_stream ins{outs.data(), outs.size()};
    IO io{ins};

    TestFlagsCpp i1;
    TestFlagsCpp i2;
    TestFlagsCpp i3;
    std::vector<TestFlagsCpp> v;

    io(i1, i2, i3, v);

    ASSERT_EQ(i1, TestFlagsCpp::One);
    ASSERT_EQ(i2, TestFlagsCpp::Two);
    ASSERT_EQ(i3, TestFlagsCpp::Three);
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(v[0], TestFlagsCpp::One);
    ASSERT_EQ(v[1], TestFlagsCpp::Two);
    ASSERT_EQ(v[2], TestFlagsCpp::Three);
}

TEST(Serialization, PodArray) {
    output_memory_stream output{0xFFu};
    {
        IO io{output};
        std::vector<Vector2<double>> ds{
                {0.1, -0.2},
                {0.3, -0.4}
        };

        std::vector<Vector2<float>> fs{
                {0.5f, 0.6f},
                {0.7f, 0.8f}
        };
        io(ds, fs);
        ASSERT_EQ(output.size(), 4 + 4 * sizeof(double) + 4 + 4 * sizeof(float));
    }

    input_memory_stream input{output.data(), output.size()};
    {
        std::vector<Vector2<double>> ds{
                {1, 1},
                {1, 1},
                {1, 1},
                {1, 1},
                {1, 1},
                {1, 1}
        }; // io should reset size
        std::vector<Vector2<float>> fs;

        IO io{input};
        io(ds, fs);

        ASSERT_EQ(ds.size(), 2);
        ASSERT_FLOAT_EQ(ds[0].x, 0.1);
        ASSERT_FLOAT_EQ(ds[0].y, -0.2);
        ASSERT_FLOAT_EQ(ds[1].x, 0.3);
        ASSERT_FLOAT_EQ(ds[1].y, -0.4);

        ASSERT_EQ(fs.size(), 2);
        ASSERT_FLOAT_EQ(fs[1].x, 0.7);
    }
}