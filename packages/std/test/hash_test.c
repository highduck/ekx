#include <doctest.h>
#include <ek/pre.h>
#include <ek/hash.h>
#include <ek/assert.h>

TEST_SUITE_BEGIN("hash");

uint32_t test_unaligned_murmur3_32(const void* key, uint32_t len, uint32_t seed) {
    void* mem = malloc(len);
    EK_ASSERT(mem);
    memcpy(mem, key, len);
    const uint32_t result = hash_murmur3_32(mem, len, seed);
    free(mem);
    return result;
}

#define hash_murmur3_32_s(str, seed) (test_unaligned_murmur3_32((str), sizeof(str)-1u, (seed)))

TEST_CASE("hash_murmur3_32") {
    /*
     * reference: https://stackoverflow.com/a/31929528
     *
     * murmur3 32-bit test vectors
     * __________________________________________
     * | Input        | Seed       | Expected   |
     * |--------------|------------|------------|
     * | (no bytes)   | 0          | 0          | with zero data and zero seed, everything becomes zero
     * | (no bytes)   | 1          | 0x514E28B7 | ignores nearly all the math
     * | (no bytes)   | 0xffffffff | 0x81F16F39 | make sure your seed uses unsigned 32-bit math
     * | FF FF FF FF  | 0          | 0x76293B50 | make sure 4-byte chunks use unsigned math
     * | 21 43 65 87  | 0          | 0xF55B516B | Endian order. UInt32 should end up as 0x87654321
     * | 21 43 65 87  | 0x5082EDEE | 0x2362F9DE | Special seed value eliminates initial key with xor
     * | 21 43 65     | 0          | 0x7E4A8634 | Only three bytes. Should end up as 0x654321
     * | 21 43        | 0          | 0xA0F7B07A | Only two bytes. Should end up as 0x4321
     * | 21           | 0          | 0x72661CF4 | Only one byte. Should end up as 0x21
     * | 00 00 00 00  | 0          | 0x2362F9DE | Make sure compiler doesn't see zero and convert to null
     * | 00 00 00     | 0          | 0x85F0B427 |
     * | 00 00        | 0          | 0x30F4C306 |
     * | 00           | 0          | 0x514E28B7 |
     */

    CHECK_EQ(test_unaligned_murmur3_32(0, 0, 0), 0);
    CHECK_EQ(test_unaligned_murmur3_32(0, 0, 1), 0x514E28B7u);
    CHECK_EQ(test_unaligned_murmur3_32(0, 0, 0xffffffff), 0x81F16F39u);
    CHECK_EQ(test_unaligned_murmur3_32("\xFF\xFF\xFF\xFF", 4, 0), 0x76293B50u);
    CHECK_EQ(test_unaligned_murmur3_32("\x21\x43\x65\x87", 4, 0), 0xF55B516Bu);
    CHECK_EQ(test_unaligned_murmur3_32("\x21\x43\x65\x87", 4, 0x5082EDEE), 0x2362F9DEu);
    CHECK_EQ(test_unaligned_murmur3_32("\x21\x43\x65", 3, 0), 0x7E4A8634u);
    CHECK_EQ(test_unaligned_murmur3_32("\x21\x43", 2, 0), 0xA0F7B07Au);
    CHECK_EQ(test_unaligned_murmur3_32("\x21", 1, 0), 0x72661CF4u);
    CHECK_EQ(test_unaligned_murmur3_32("\x00\x00\x00\x00", 4, 0), 0x2362F9DEu);
    CHECK_EQ(test_unaligned_murmur3_32("\x00\x00\x00", 3, 0), 0x85F0B427u);
    CHECK_EQ(test_unaligned_murmur3_32("\x00\x00", 2, 0), 0x30F4C306u);
    CHECK_EQ(test_unaligned_murmur3_32("\x00", 1, 0), 0x514E28B7u);

    CHECK_EQ(hash_murmur3_32_s("", 0), 0); //empty string with zero seed should give zero
    CHECK_EQ(hash_murmur3_32_s("", 1), 0x514E28B7);
    CHECK_EQ(hash_murmur3_32_s("", 0xffffffff), 0x81F16F39); //make sure seed value is handled unsigned
    CHECK_EQ(hash_murmur3_32_s("\0\0\0\0", 0), 0x2362F9DE); //make sure we handle embedded nulls

    CHECK_EQ(hash_murmur3_32_s("aaaa", 0x9747b28c), 0x5A97808A); //one full chunk
    CHECK_EQ(hash_murmur3_32_s("aaa", 0x9747b28c), 0x283E0130); //three characters
    CHECK_EQ(hash_murmur3_32_s("aa", 0x9747b28c), 0x5D211726); //two characters
    CHECK_EQ(hash_murmur3_32_s("a", 0x9747b28c), 0x7FA09EA6); //one character

    //Endian order within the chunks
    CHECK_EQ(hash_murmur3_32_s("abcd", 0x9747b28c), 0xF0478627); //one full chunk
    CHECK_EQ(hash_murmur3_32_s("abc", 0x9747b28c), 0xC84A62DD);
    CHECK_EQ(hash_murmur3_32_s("ab", 0x9747b28c), 0x74875592);
    CHECK_EQ(hash_murmur3_32_s("a", 0x9747b28c), 0x7FA09EA6);

    CHECK_EQ(hash_murmur3_32_s("Hello, world!", 0x9747b28c), 0x24884CBA);

    //Make sure you handle UTF-8 high characters. A bcrypt implementation messed this up
    CHECK_EQ(hash_murmur3_32_s("ππππππππ", 0x9747b28c), 0xD58063C1); //U+03C0: Greek Small Letter Pi

    //String of 256 characters.
    //Make sure you don't store string lengths in a char, and overflow at 255 bytes (as OpenBSD's canonical BCrypt implementation did)
    CHECK_EQ(hash_murmur3_32_s(
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
            0x9747b28c),
             0x37405BDC);

    CHECK_EQ(hash_murmur3_32_s("abc", 0), 0xB3DD93FA);
    CHECK_EQ(hash_murmur3_32_s("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 0), 0xEE925B90);
    CHECK_EQ(hash_murmur3_32_s("The quick brown fox jumps over the lazy dog", 0x9747b28c), 0x2FA826CD);
}

TEST_CASE("inline hash") {
    H("1");
    H("1");
    H("1");
    H("2");
    H("2");
    H("2");
    H("3");
    H("3");
    H("4");

    CHECK_EQ(hsp_get(H("1")), "1");
    CHECK_EQ(hsp_get(H("not_added")), "not_added");
    CHECK_EQ(hsp_get(0), nullptr);

}

TEST_SUITE_END();
