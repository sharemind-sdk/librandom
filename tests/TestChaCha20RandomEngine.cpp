#include "../src/ChaCha20RandomEngine.h"

#include <array>
#include <sharemind/TestAssert.h>


using namespace sharemind;

int main() {
    // Test data taken from RFC7539 Section-2.4.2.

    const std::array<uint8_t, ChaCha20RandomEngine::SeedSize> seed {{
        // Key:
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
        0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
        // Nonce:
        0x0, 0x0, 0x0,0x4a, 0x0, 0x0, 0x0, 0x0 }};

    const std::array<uint32_t, 16> firstBlock {{
        0xf3514f22, 0xe1d91b40, 0x6f27de2f, 0xed1d63b8,
        0x821f138c, 0xe2062c3d, 0xecca4f7e, 0x78cff39e,
        0xa30a3b8a, 0x920a6072, 0xcd7479b5, 0x34932bed,
        0x40ba4c79, 0xcd343ec6, 0x4c2c21ea, 0xb7417df0 }};

    const std::array<uint32_t, 16> secondBlock {{
        0x9f74a669, 0x410f633f, 0x28feca22, 0x7ec44dec,
        0x6d34d426, 0x738cb970, 0x3ac5e9f3, 0x45590cc4,
        0xda6e8b39, 0x892c831a, 0xcdea67c1, 0x2b7e1d90,
        0x037463f3, 0xa11a2073, 0xe8bcfb88, 0xedc49139 }};

    ChaCha20RandomEngine engine {seed.data()};

    // We generate 4 blocks at a time
    for (size_t i = 0; i < 16u; ++ i) {
        const uint32_t v1 = engine.randomValue<uint32_t>(); // counter = 0
        const uint32_t v2 = engine.randomValue<uint32_t>(); // counter = 1
        const uint32_t v3 = engine.randomValue<uint32_t>(); // counter = 2
        const uint32_t v4 = engine.randomValue<uint32_t>(); // counter = 3
        (void) v1; (void) v2; (void) v3; (void) v4;
        SHAREMIND_TESTASSERT(firstBlock[i] == v2);
        SHAREMIND_TESTASSERT(secondBlock[i] == v3);
    }

    return 0;
}
