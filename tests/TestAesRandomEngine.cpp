#include "../src/AesRandomEngine.h"

#include <array>
#include <iostream>

#define AES_TEST_BLOCK_SIZE 16
#define AES_TEST_SEED_SIZE 32

using AesSeed = std::array<uint8_t, AES_TEST_SEED_SIZE>;
using AesBlock = std::array<uint8_t, AES_TEST_BLOCK_SIZE>;

using namespace sharemind;

void test1 () {
    // Outer seed:
    const AesSeed seed {{
        // Key:
        0xAE, 0x68, 0x52, 0xF8, 0x12, 0x10, 0x67, 0xCC,
        0x4B, 0xF7, 0xA5, 0x76, 0x55, 0x77, 0xF3, 0x9E,
        // IV:
        0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }};

    // Inner seed:
    // b7603328dbc2931b410e16c8067e62df
    // 52e82dd83e0f4a7f3761d34328f2c381
    
    // Generated blocks:
    // 6b3656e7f3bc391fe29d549073cebd66
    // f40c77105b107c3f5ad772398ed112b3

    const AesBlock firstBlock {{
        0x6b, 0x36, 0x56, 0xe7, 0xf3, 0xbc, 0x39, 0x1f,
        0xe2, 0x9d, 0x54, 0x90, 0x73, 0xce, 0xbd, 0x66
    }};

    const AesBlock secondBlock {{
        0xf4, 0x0c, 0x77, 0x10, 0x5b, 0x10, 0x7c, 0x3f,
        0x5a, 0xd7, 0x72, 0x39, 0x8e, 0xd1, 0x12, 0xb3, 
    }};

    AesRandomEngine engine {seed.data()};

    for (uint8_t c : firstBlock) {
        const uint8_t v = engine.randomValue<uint8_t>();
        assert (c == v);
    }

    std::array<uint8_t, 16> tempBlock;
    engine.fillBytes(tempBlock.data(), tempBlock.size());
    assert (tempBlock == secondBlock);
}

void test2 () {

    // Outer state:
    const AesSeed seed {{
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
    }};

    // Inner state:
    // ec8cdf7398607cb0f2d21675ea9ea1e4
    // 362b7c3c6773516318a077d7fc5073ae
    
    AesBlock testBlock;

    AesRandomEngine engine {seed.data()};

    // Jump ahead by 32 blocks.
    for (size_t i = 0; i < 32; ++ i) {
        engine.fillBytes(testBlock.data(), testBlock.size());
    }

    // Inner state:
    // ec8cdf7398607cb0f2d21675ea9ea1e4
    // 362b7c3c6773516318a077d7fc5073ce 

    const AesBlock firstBlock {{
        0xcc, 0x63, 0x39, 0x2f, 0x12, 0x2c, 0x8a, 0x38,
        0xf4, 0x1a, 0x5d, 0xc6, 0x70, 0x81, 0xe9, 0xa3
    }};

    engine.fillBytes(testBlock.data(), testBlock.size());
    assert (testBlock == firstBlock);

    const AesBlock secondBlock {{
        0xe4, 0x56, 0x62, 0x7f, 0x90, 0x84, 0xb2, 0xc4,
        0xbf, 0x30, 0x63, 0x6b, 0xf0, 0x85, 0xdf, 0x31
    }};

    engine.fillBytes(testBlock.data(), testBlock.size());
    assert (testBlock == secondBlock);

    const AesBlock thirdBlock {{
        0xae, 0xea, 0x28, 0xdc, 0xd7, 0x71, 0xad, 0x27,
        0x2d, 0x36, 0x2e, 0xff, 0x82, 0x24, 0xb0, 0xe7
    }};

    engine.fillBytes(testBlock.data(), testBlock.size());
    assert (testBlock == thirdBlock);
}

int main () {
    assert (AesRandomEngine::seedSize() == AES_TEST_SEED_SIZE);
    test1();
    test2();
    return 0;
}
