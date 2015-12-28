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

#include "SNOW2RandomEngine.h"

#include "RandomEngine.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif


namespace /* anonymous */ {

// Intentionally included inside anonymous namespace!
#include "snow2tab.h"

using namespace sharemind;

struct Inner {

/* Methods: */

    inline Inner(const void * const memptr) noexcept;

    /*
     * Function:  snow_loadkey_fast
     *
     * Synopsis:
     *   Loads the key material and performs the initial mixing.
     *
     * Returns: void
     *
     * Assumptions:
     *   keysize is either 128 or 256.
     *   key is of proper length, for keysize=128, key is of lenght 16 bytes
     *      and for keysize=256, key is of length 32 bytes.
     *   key is given in big endian format,
     *   For 128 bit key:
     *        key[0]-> msb of k_3
     *         ...
     *        key[3]-> lsb of k_3
     *         ...
     *        key[12]-> msb of k_0
     *         ...
     *        key[15]-> lsb of k_0
     *
     *   For 256 bit key:
     *        key[0]-> msb of k_7
     *          ...
     *        key[3]-> lsb of k_7
     *          ...
     *        key[28]-> msb of k_0
     *          ...
     *        key[31]-> lsb of k_0
     *
     * Authors:
     * Patrik Ekdahl & Thomas Johansson
     * Dept. of Information Technology
     * P.O. Box 118
     * SE-221 00 Lund, Sweden,
     * email: {patrik,thomas}@it.lth.se
     */
    template <uint32_t KEYSIZE>
    inline void snow_loadkey_fast_p(Snow2Iv const & iv,
                                    Snow2Key const & key) noexcept;

    inline void snow_loadkey_fast_p_common(Snow2Iv const & iv) noexcept;

    inline void snow_keystream_fast_p() noexcept;

    static constexpr inline uint32_t ainv_mul(uint32_t const v) noexcept
    { return (v >> 8) ^ snow_alphainv_mul[v & 0xff]; }

    static constexpr inline uint32_t a_mul(uint32_t const v) noexcept
    { return (v << 8) ^ snow_alpha_mul[v >> 24]; }

    inline void newRs(unsigned const sIndex) noexcept {
        assert(sIndex < 16u);
        uint32_t const fsmtmp = r2 + s[sIndex];
        r2 = snow_T0[r1 & 0xff]
           ^ snow_T1[(r1 >> 8) & 0xff]
           ^ snow_T2[(r1 >> 8) & 0xff]
           ^ snow_T3[r1 >> 24];
        r1 = fsmtmp;
    }

    template <unsigned offset>
    static inline uint32_t keyShift(Snow2Key const & key) noexcept {
        static_assert(
                offset < SNOW2RandomEngine::SizeOfArrayInBytes<Snow2Key>::value,
                "");
        static_assert((offset % 4u) == 0u, "");
        return (uint32_t{key[offset     ]} << 24) |
               (uint32_t{key[offset + 1u]} << 16) |
               (uint32_t{key[offset + 2u]} <<  8) |
                uint32_t{key[offset + 3u]};
    }

    inline void fillBytes(void * memptr, size_t size) noexcept;

/* Fields: */

    std::array<uint32_t, 16u> s;
    uint32_t r1, r2;
    union {
        std::array<uint32_t, 16u> keystream;
        std::array<uint8_t, sizeof(uint32_t) * 16> un_byte_keystream;
    };
    static_assert(sizeof(keystream) == 64u, "");
    static_assert(sizeof(un_byte_keystream) == 64u, "");
    unsigned haveData = 0u;

};

template <>
inline void Inner::snow_loadkey_fast_p<128u>(Snow2Iv const & iv,
                                             Snow2Key const & key) noexcept
{
    s[15u] = keyShift<0u>(key);
    s[14u] = keyShift<4u>(key);
    s[13u] = keyShift<8u>(key);
    s[12u] = keyShift<12u>(key);
    s[11u] = ~s[15u]; /* bitwise inverse */
    s[10u] = ~s[14u];
    s[ 9u] = ~s[13u];
    s[ 8u] = ~s[12u];
    s[ 7u] =  s[15u]; /* just copy */
    s[ 6u] =  s[14u];
    s[ 5u] =  s[13u];
    s[ 4u] =  s[12u];
    s[ 3u] = ~s[15u]; /* bitwise inverse */
    s[ 2u] = ~s[14u];
    s[ 1u] = ~s[13u];
    s[ 0u] = ~s[12u];
    return snow_loadkey_fast_p_common(iv);
}


template <>
inline void Inner::snow_loadkey_fast_p<256u>(Snow2Iv const & iv,
                                             Snow2Key const & key) noexcept
{
    s[15u] = keyShift<0u>(key);
    s[14u] = keyShift<4u>(key);
    s[13u] = keyShift<8u>(key);
    s[12u] = keyShift<12u>(key);
    s[11u] = keyShift<16u>(key);
    s[10u] = keyShift<20u>(key);
    s[ 9u] = keyShift<24u>(key);
    s[ 8u] = keyShift<28u>(key);
    s[ 7u] = ~s[15u]; /* bitwise inverse */
    s[ 6u] = ~s[14u];
    s[ 5u] = ~s[13u];
    s[ 4u] = ~s[12u];
    s[ 3u] = ~s[11u];
    s[ 2u] = ~s[10u];
    s[ 1u] = ~s[ 9u];
    s[ 0u] = ~s[ 8u];
    return snow_loadkey_fast_p_common(iv);
}

inline Inner::Inner(const void * const memptr) noexcept {
    #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
    VALGRIND_MAKE_MEM_DEFINED(this, sizeof(Inner));
    #endif

    Snow2Key snowkey;
    Snow2Iv iv;

    memcpy(snowkey.data(), memptr, snowkey.size());
    memcpy(iv.data(), ptrAdd(memptr, snowkey.size()), iv.size());
    snow_loadkey_fast_p<256u>(iv, snowkey);
}

inline void Inner::snow_loadkey_fast_p_common(Snow2Iv const & iv) noexcept {
    /* XOR IV values */
    s[15u] ^= iv[0u];
    s[12u] ^= iv[1u];
    s[10u] ^= iv[2u];
    s[ 9u] ^= iv[3u];

    r1 = 0u;
    r2 = 0u;

    /* Do 32 initial clockings */
    for(int loopCounter = 0; loopCounter < 2; loopCounter++) {
        for (unsigned i = 0u; i < 16u; i++) {
            s[i] = a_mul(s[i])
                 ^ s[(i + 2u) % 16u]
                 ^ ainv_mul(s[(i + 11u) % 16u])
                 ^ (r1 + s[(i + 15u) % 16u])
                 ^ r2;
            newRs((i + 5u) % 16u);
        }
    }
}

/*
 * Function: snow_keystream_fast
 *
 * Synopsis:
 *   Clocks the cipher 16 times and returns 16 words of keystream symbols
 *   in keystream.
 *
 * Returns: void
 *
 * Authors:
 * Patrik Ekdahl & Thomas Johansson
 * Dept. of Information Technology
 * P.O. Box 118
 * SE-221 00 Lund, Sweden,
 * email: {patrik,thomas}@it.lth.se
 *
 */
inline void Inner::snow_keystream_fast_p() noexcept {
    for (unsigned i = 0u; i < 16u; i++) {
        s[i] = a_mul(s[i]) ^ s[(i + 2u) % 16u] ^ ainv_mul(s[(i + 11u) % 16u]);
        newRs((i + 5u) % 16u);
        keystream[i] = (r1 + s[i]) ^ r2 ^ s[(i + 1u) % 16u];
    }
    haveData = sizeof(keystream);
}

void Inner::fillBytes(void * memptr, size_t size) noexcept {
    if (size <= 0u)
        return;
    assert(memptr);
    static constexpr size_t const maxBytes = sizeof(un_byte_keystream);
    assert(haveData <= maxBytes);

    // Fill leftover data from last time and generate new data if needed:
    if (haveData < maxBytes) {
        auto const * const readPtr = &un_byte_keystream[maxBytes - haveData];
        if (size <= haveData) {
            memcpy(memptr, readPtr, size);
            haveData -= size;
            return;
        }
        memcpy(memptr, readPtr, haveData);
        memptr = ptrAdd(memptr, haveData);
        size -= haveData;
        snow_keystream_fast_p();
    }
    assert(haveData == maxBytes);

    // Fill big chunks (except last one it that one is big as well):
    while (size > maxBytes) {
        memcpy(memptr, un_byte_keystream.data(), maxBytes);
        memptr = ptrAdd(memptr, maxBytes);
        size -= maxBytes;
        snow_keystream_fast_p();
        assert(haveData == maxBytes);
    }

    // Fill the rest:
    memcpy(memptr, un_byte_keystream.data(), size);
    haveData = maxBytes - size;
}

extern "C"
void SNOW2RandomEngine_fill_bytes(SharemindRandomEngine * rng_,
                                  void * memptr,
                                  size_t size)
{
    assert(rng_);
    return SNOW2RandomEngine::fromWrapper(*rng_).fillBytes(memptr, size);
}

extern "C"
void SNOW2RandomEngine_free(SharemindRandomEngine * rng_) {
    assert(rng_);
    delete &SNOW2RandomEngine::fromWrapper(*rng_);
}

} // namespace anonymous

namespace sharemind {

SNOW2RandomEngine::SNOW2RandomEngine(const void * const memptr)
    : SharemindRandomEngine {
          &SNOW2RandomEngine_fill_bytes,
          &SNOW2RandomEngine_free
      }
    , m_inner{new Inner{memptr}}
{}

SNOW2RandomEngine::~SNOW2RandomEngine() noexcept
{ delete static_cast<Inner *>(m_inner); }

void SNOW2RandomEngine::fillBytes(void * memptr, size_t size) noexcept
{ return static_cast<Inner *>(m_inner)->fillBytes(memptr, size); }

} // namespace sharemind {
