#include <unit.h>

#include <ek/serialize/serialize.hpp>
#include <ek/serialize/stl/Vector.hpp>
#include <ek/serialize/stl/String.hpp>
#include <ek/serialize/stl/Array.hpp>

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

    Vector2(T px, T py) : x{px}, y{py} {
    }
};

template<typename T>
struct declared_as_pod_type<Vector2<T>> : public std::true_type {
};

SUITE(cxx_serialize) {

    IT("Basic") {
        Bar bar{};

        output_memory_stream writeStream{200};
        IO writer{writeStream};

        std::string emptyString;
        auto ff = TestFlagsCpp::Three;
        writer(bar, ff, emptyString);

//    writer(bar, TestFlagsCpp::Three);

        // Bar: 17
        // TestFlagsCpp: 1
        // empty string: 4 + 1 (string length + null-terminator byte)
        REQUIRE_EQ(writeStream.size(), 17 + 1 + (4 + 1));

        input_memory_stream read1{writeStream.data(), writeStream.size()};
        IO io{read1};

        Bar barLoaded{};
        TestFlagsCpp f{};
        std::string str{"hi"};
        io(barLoaded, f, str);
        REQUIRE_EQ(barLoaded.f1, 2.0f); // near
        REQUIRE_EQ(barLoaded.d3, 3); // near
        REQUIRE_EQ(barLoaded.i4, 4);
        REQUIRE_EQ(barLoaded.flag, true);
        REQUIRE(f == TestFlagsCpp::Three);
        REQUIRE(str.empty());

        input_memory_stream read2{writeStream.data(), writeStream.size()};
        IO<input_memory_stream> io2{read2};

        Bar barCorrupted{};
        io2(barCorrupted);
        REQUIRE_EQ(barLoaded.f1, (2.0f)); //
        REQUIRE_EQ(barLoaded.d3, (3)); //
        REQUIRE_EQ(barLoaded.i4, 4);
    }

    IT("Basic2") {

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

        REQUIRE_EQ(s.advanceWidth, -3);
        REQUIRE_EQ(s.sprite.c_str(), "text");
        REQUIRE_EQ(s.box[3], 3);
        REQUIRE_EQ(s.codes.size(), 3);
        REQUIRE_EQ(s.codes[0], 1);
    }

    IT("Nesting") {

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

        REQUIRE_EQ(s.glyphs.size(), 2);
        REQUIRE(s.glyphs[0].sprite == glyph1.sprite);
        REQUIRE(s.glyphs[0].box == glyph1.box);
        REQUIRE(s.glyphs[0].codes == glyph1.codes);
        REQUIRE(s.glyphs[1].sprite == glyph2.sprite);
        REQUIRE(s.glyphs[1].box == glyph2.box);
        REQUIRE(s.glyphs[1].codes == glyph2.codes);
        REQUIRE_EQ(s.unitsPerEm, 2048);
        REQUIRE_EQ(s.sizes[3], 32);
    }

    IT("Enum") {
        output_memory_stream outs{200};
        IO writer{outs};

        TestFlagsCpp a{TestFlagsCpp::One};
        TestFlagsCpp b{TestFlagsCpp::Two};
        TestFlagsCpp c{TestFlagsCpp::Three};
        std::vector<TestFlagsCpp> d{TestFlagsCpp::One, TestFlagsCpp::Two, TestFlagsCpp::Three};
        writer(a, b, c, d);

        REQUIRE_EQ(outs.size(), 3 + 4 + 3);

        input_memory_stream ins{outs.data(), outs.size()};
        IO io{ins};

        TestFlagsCpp i1;
        TestFlagsCpp i2;
        TestFlagsCpp i3;
        std::vector<TestFlagsCpp> v;

        io(i1, i2, i3, v);

        REQUIRE(i1 == TestFlagsCpp::One);
        REQUIRE(i2 == TestFlagsCpp::Two);
        REQUIRE(i3 == TestFlagsCpp::Three);
        REQUIRE_EQ(v.size(), 3);
        REQUIRE(v[0] == TestFlagsCpp::One);
        REQUIRE(v[1] == TestFlagsCpp::Two);
        REQUIRE(v[2] == TestFlagsCpp::Three);
    }

    IT("PodArray") {
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
            REQUIRE_EQ(output.size(), 4 + 4 * sizeof(double) + 4 + 4 * sizeof(float));
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

            REQUIRE_EQ(ds.size(), 2);
            REQUIRE_EQ(ds[0].x, (0.1)); //
            REQUIRE_EQ(ds[0].y, (-0.2)); //
            REQUIRE_EQ(ds[1].x, (0.3)); //
            REQUIRE_EQ(ds[1].y, (-0.4)); //

            REQUIRE_EQ(fs.size(), 2);
            REQUIRE_EQ(fs[1].x, (0.7f)); //
        }
    }
}
