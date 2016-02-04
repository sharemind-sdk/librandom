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
 * NOTE: XXX: We diverge from the standard in few aspects:
 * 1. We use 64-bit counter as opposed to 32-bit counter. This is because
 *    generating 256G of randomness is just an ordinary tuesday for Sharemind.
 * 2. Counter is initialized to 0 as oppsed to 1. This avoids overflow checks.
 * 3. We compute 4 blocks in parallel and return those blocks interleaved. This
 *    does not matter as every byte of chacha20 output should look uniformly
 *    random. First 32-bit value returned is generated with counter = 0, the second
 *    with counter = 1 and so forth. Now, fourth 32-bit value is again generated
 *    with counter = 0. We do this to avoid transposing data and instead use a
 *    single memcpy. After 4x16 32-bit values have been generated the counter
 *    is incremented by 4.
 */

#include "ChaCha20RandomEngine.h"

#include <cassert>
#include <cstring>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#if SHAREMIND_HAVE_EMMINTRIN_SSE2
#include <emmintrin.h>
#endif

namespace sharemind {

namespace /* anonymous */ {

#if SHAREMIND_HAVE_EMMINTRIN_SSE2
using v4_u32_t = __m128i;

inline
v4_u32_t v4_set(uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept {
    return _mm_set_epi32(w, z, y, x);
}

inline v4_u32_t v4_set1(const uint32_t x) noexcept {
    return _mm_set1_epi32(x);
}

inline void v4_rotl (v4_u32_t& v, const uint32_t n) noexcept {
    const uint32_t m = sizeof(uint32_t) * 8u - n;
    v = _mm_or_si128(_mm_slli_epi32(v, n), _mm_srli_epi32(v, m));
}

inline void v4_add(v4_u32_t& v, const v4_u32_t& u) noexcept {
    v = _mm_add_epi32(v, u);
}

inline void v4_xor(v4_u32_t& v, const v4_u32_t& u) noexcept {
    v = _mm_xor_si128(v, u);
}

#else

#warning Using ~4x as slow ChaCha20 due to missing SSE2 support.

struct v4_u32_t { uint32_t v0, v1, v2, v3; };

inline
v4_u32_t v4_set(uint32_t x, uint32_t y, uint32_t z, uint32_t w) noexcept {
    return v4_u32_t {x, y, z, w};
}

inline v4_u32_t v4_set1(const uint32_t x) noexcept {
    return v4_u32_t {x, x, x, x};
}

inline void v4_rotl (v4_u32_t& v, const uint32_t n) noexcept {
    const uint32_t m = sizeof(uint32_t) * 8u - n;
    v.v0 = (v.v0 << n) | (v.v0 >> m);
    v.v1 = (v.v1 << n) | (v.v1 >> m);
    v.v2 = (v.v2 << n) | (v.v2 >> m);
    v.v3 = (v.v3 << n) | (v.v3 >> m);
}

inline void v4_add(v4_u32_t& v, const v4_u32_t& u) noexcept {
    v.v0 += u.v0; v.v1 += u.v1; v.v2 += u.v2; v.v3 += u.v3;
}

inline void v4_xor(v4_u32_t& v, const v4_u32_t& u) noexcept {
    v.v0 ^= u.v0; v.v1 ^= u.v1; v.v2 ^= u.v2; v.v3 ^= u.v3;
}

#endif

static_assert(sizeof(v4_u32_t) == 4*sizeof(uint32_t),
              "Incorrect size of v4_u32_t");

static_assert(sizeof(uint32_t) <= sizeof(size_t),
              "uint32_t bigger than size_t.");

inline uint32_t u8to32_little (const uint8_t* p) noexcept {
    const uint32_t p0 = p[0];
    const uint32_t p1 = p[1];
    const uint32_t p2 = p[2];
    const uint32_t p3 = p[3];
    return p0 | (p1 << 8) | (p2 << 16) | (p3 << 24);
}

} // namespace anonymous

ChaCha20RandomEngine::ChaCha20RandomEngine(void const * seed) noexcept {
    assert(seed);
    #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
    VALGRIND_MAKE_MEM_DEFINED(this, sizeof(ChaCha20RandomEngine));
    #endif

    uint8_t key[CHACHA20_KEY_SIZE];
    uint8_t nonce[CHACHA20_NONCE_SIZE];
    memcpy(key, seed, CHACHA20_KEY_SIZE);
    memcpy(nonce, ptrAdd(seed, CHACHA20_KEY_SIZE), CHACHA20_NONCE_SIZE);

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
        v4_add(x[a], x[b]); v4_xor(x[d], x[a]); v4_rotl(x[d], 16); \
        v4_add(x[c], x[d]); v4_xor(x[b], x[c]); v4_rotl(x[b], 12); \
        v4_add(x[a], x[b]); v4_xor(x[d], x[a]); v4_rotl(x[d],  8); \
        v4_add(x[c], x[d]); v4_xor(x[b], x[c]); v4_rotl(x[b],  7); \
    } while (0)

void ChaCha20RandomEngine::fillBytes(void * buffer, size_t size) noexcept
{
    if (size == 0u)
        return;
    assert(buffer);

    size_t unconsumedSize = CHACHA20_BUFFER_SIZE - m_consumed_byte_count;
    size_t offsetStart = 0;
    size_t offsetEnd = unconsumedSize;

    // Consume full blocks (first might already be partially or entirely consumed).
    while (offsetEnd <= size) {
        memcpy(ptrAdd(buffer, offsetStart), &m_block[m_consumed_byte_count], unconsumedSize);
        { // Generate next four blocks:
            v4_u32_t x[16];

            for (size_t i = 0; i < 16u; ++ i) {
                x[i] = v4_set1(m_state[i]);
            }

            v4_add(x[12], v4_set(0, 1, 2, 3));

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
                v4_add(x[i], v4_set1(m_state[i]));
            }

            v4_add(x[12], v4_set(0, 1, 2, 3));

            // Ordering of bytes does not matter for us:
            memcpy(&m_block[0], &x[0], CHACHA20_BUFFER_SIZE);

            /* Increment the counter.
             *
             * NOTE: This is one place where our implementation differs from
             * RFC7539 where only the m_state[12] is used as a counter. This
             * limits the RNG to generating only 256 GB of data. Thus, we
             * borrow m_state[13] from the nonce and use it as higher bits of
             * the counter.
             *
             * NOTE: This does not overflow as long as the m_state[12] is
             * initially 0 and the number of values in uint32_t is divisible
             * by 4!
             */
            m_state[12] += 4;
            if (m_state[12] == 0) {
                m_state[13] ++;
            }
        }

        m_consumed_byte_count = 0;
        unconsumedSize = CHACHA20_BUFFER_SIZE;
        offsetStart = offsetEnd;
        offsetEnd += CHACHA20_BUFFER_SIZE;
    }

    const size_t remainingSize = size - offsetStart;
    memcpy(ptrAdd(buffer, offsetStart), &m_block[m_consumed_byte_count], remainingSize);
    m_consumed_byte_count += remainingSize;
    assert(m_consumed_byte_count <= CHACHA20_BUFFER_SIZE); // the supply may deplete
}

} // namespace sharemind {
