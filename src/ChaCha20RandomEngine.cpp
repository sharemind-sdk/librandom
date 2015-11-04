/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

/*
 * Implementation is based on RFC7539:
 *      https://tools.ietf.org/html/rfc7539
 *
 * We diverge from the standard in one aspect. Namely we use 64-bit counter
 * as opposed to 32-bit counter. This is because generating 256G of randomness
 * is just an ordinary tuesday for Sharemind.
 *
 * TODO: Compute 4 blocks in parallel. This should improve performance at least 3x.
 */

#include "ChaCha20RandomEngine.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <openssl/rand.h>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#define CHACHA20_KEY_SIZE 32
#define CHACHA20_BLOCK_SIZE 64
#define CHACHA20_NONCE_SIZE 8

using namespace sharemind;

namespace /* anonymouse */ {

inline uint32_t rotl32 (uint32_t v, uint8_t n) noexcept {
    return (v << n) | (v >> (sizeof(v) * 8u - n));
}

inline uint32_t u8to32_little (const uint8_t* p) noexcept {
    const uint32_t p0 = p[0];
    const uint32_t p1 = p[1];
    const uint32_t p2 = p[2];
    const uint32_t p3 = p[3];
    return p0 | (p1 << 8) | (p2 << 16) | (p3 << 24);
}

extern "C" {
void ChaCha20RandomEngine_fill_bytes(SharemindRandomEngine* rng_, void * memptr_, size_t size);
void ChaCha20RandomEngine_free(SharemindRandomEngine* rng_);
}

class ChaCha20RandomEngine : public SharemindRandomEngine {
public: /* Methods: */

    inline explicit ChaCha20RandomEngine(const void* memptr_) noexcept
        : SharemindRandomEngine {
              ChaCha20RandomEngine_fill_bytes,
              ChaCha20RandomEngine_free
          }
        , m_block_consumed (CHACHA20_BLOCK_SIZE)
    {
        #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
        VALGRIND_MAKE_MEM_DEFINED(this, sizeof(ChaCha20RandomEngine));
        #endif

        assert (memptr_ != nullptr);
        const auto memptr = static_cast<const uint8_t*>(memptr_);
        chacha20_load_key(&memptr[0], &memptr[0] + CHACHA20_KEY_SIZE);
    }

    inline static ChaCha20RandomEngine& fromWrapper(SharemindRandomEngine& base) noexcept {
        return static_cast<ChaCha20RandomEngine&>(base);
    }

    inline void chacha20_load_key(const uint8_t* key, const uint8_t* nonce) noexcept;

    inline void chacha20_next_block() noexcept;

public: /* Fields: */

    uint32_t m_state[16]; // internal state of the ChaCha20 cipher
    size_t m_block_consumed; // number of bytes that have been consumed from the block
    uint8_t m_block[CHACHA20_BLOCK_SIZE]; // a single generated block
};

inline void ChaCha20RandomEngine::chacha20_load_key(const uint8_t* key,  const uint8_t* nonce) noexcept {
    m_state[0]  = 0x61707865;
    m_state[1]  = 0x3320646e;
    m_state[2]  = 0x79622d32;
    m_state[3]  = 0x6b206574;
    m_state[4]  = u8to32_little(key + 0);
    m_state[5]  = u8to32_little(key + 4);
    m_state[6]  = u8to32_little(key + 8);
    m_state[7]  = u8to32_little(key + 12);
    m_state[8]  = u8to32_little(key + 16);
    m_state[9]  = u8to32_little(key + 20);
    m_state[10] = u8to32_little(key + 24);
    m_state[11] = u8to32_little(key + 28);
    m_state[12] = 0;
    m_state[13] = 0;
    m_state[14] = u8to32_little(nonce + 0);
    m_state[15] = u8to32_little(nonce + 4);
}

#define QUARTERROUND(a,b,c,d) \
    do { \
        x[a] += x[b]; x[d] ^= x[a]; x[d] = rotl32(x[d], 16); \
        x[c] += x[d]; x[b] ^= x[c]; x[b] = rotl32(x[b], 12); \
        x[a] += x[b]; x[d] ^= x[a]; x[d] = rotl32(x[d],  8); \
        x[c] += x[d]; x[b] ^= x[c]; x[b] = rotl32(x[b],  7); \
    } while (0)

inline void ChaCha20RandomEngine::chacha20_next_block() noexcept {
    uint32_t x[16];

    for (size_t i = 0; i < 16; ++ i) {
        x[i] = m_state[i];
    }

    for (size_t i = 0; i < 10; ++ i) {
        QUARTERROUND(0, 4,  8, 12);
        QUARTERROUND(1, 5,  9, 13);
        QUARTERROUND(2, 6, 10, 14);
        QUARTERROUND(3, 7, 11, 15);
        QUARTERROUND(0, 5, 10, 15);
        QUARTERROUND(1, 6, 11, 12);
        QUARTERROUND(2, 7,  8, 13);
        QUARTERROUND(3, 4,  9, 14);
    }

    for (size_t i = 0; i < 16; ++ i) {
        x[i] = x[i] + m_state[i];
    }

    memcpy(&m_block[0], &x[0], CHACHA20_BLOCK_SIZE); // don't really care about the ordering here

    /* Increment the counter.
     * Note that this is the place where our implementation differs from RFC7539
     * where only the m_state[12] is used as a counter. This limits the RNG to
     * generating only 256 GB of data. Thus, we borrow m_state[13] from the nonce and
     * use it as higher bits of the counter.
     */
    m_state[12] ++;
    if (m_state[12] == 0) {
        m_state[13] ++;
    }
}

extern "C"
void ChaCha20RandomEngine_fill_bytes(SharemindRandomEngine* rng_, void * memptr_, size_t size)
{
    assert (rng_ != nullptr);
    assert (memptr_ != nullptr);

    auto& rng = ChaCha20RandomEngine::fromWrapper(*rng_);
    uint8_t* memptr = static_cast<uint8_t*>(memptr_);
    size_t unconsumedSize = CHACHA20_BLOCK_SIZE - rng.m_block_consumed;
    size_t offsetStart = 0;
    size_t offsetEnd = unconsumedSize;

    // Consume full blocks (first might already be partially or entirely consumed).
    while (offsetEnd <= size) {
        memcpy(memptr + offsetStart, &rng.m_block[rng.m_block_consumed], unconsumedSize);
        rng.chacha20_next_block();
        rng.m_block_consumed = 0;
        unconsumedSize = CHACHA20_BLOCK_SIZE;
        offsetStart = offsetEnd;
        offsetEnd += CHACHA20_BLOCK_SIZE;
    }

    const size_t remainingSize = size - offsetStart;
    memcpy(memptr + offsetStart, &rng.m_block[rng.m_block_consumed], remainingSize);
    rng.m_block_consumed += remainingSize;
    assert (rng.m_block_consumed <= CHACHA20_BLOCK_SIZE); // the supply may deplete
}

extern "C"
void ChaCha20RandomEngine_free(SharemindRandomEngine* rng_) {
    assert (rng_ != nullptr);
    delete &ChaCha20RandomEngine::fromWrapper(*rng_);
}


} // namespace anonymous

namespace sharemind {

size_t ChaCha20_random_engine_seed_size() noexcept {
    return CHACHA20_KEY_SIZE + CHACHA20_NONCE_SIZE;
}

SharemindRandomEngine* make_ChaCha20_random_engine(const void* memptr_) {
    return new ChaCha20RandomEngine {memptr_};
}

}