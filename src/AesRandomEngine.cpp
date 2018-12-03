/*
 * Copyright (C) Cybernetica
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

#include "AesRandomEngine.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <sharemind/abort.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

#define AES_BLOCK_SIZE 16

// The number of AES blocks to generate at a time.
// Because we are working in CTR mode we can generate those in parallel.
// Significant speedup over doing it a single block at a time.
#define AES_PARALLEL_BLOCKS 8

#define AES_INTERNAL_BUFFER (AES_PARALLEL_BLOCKS*AES_BLOCK_SIZE)

// The number of AES blocks after which we reseed the inner generator.
// This number has been selected to achieve less than 2^{-80} advantage.
#define AES_COUNTER_LIMIT (1u << 24u)

using namespace sharemind;
using namespace CryptoPP;

namespace /* anonymous */ {

struct Inner {

    inline Inner(const void * memptr_) noexcept
        : m_counter_inner (0)
        , block_consumed (AES_INTERNAL_BUFFER)
    {
        const uint8_t * key = static_cast<const uint8_t *>(memptr_);
        oPrng.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, key + AES::DEFAULT_KEYLENGTH, AES::BLOCKSIZE);
        aesReseedInner();
    }

    ~Inner() noexcept {
    }

    void aesReseedInner() noexcept;
    void aesNextBlock() noexcept;

    CTR_Mode<AES>::Encryption iPrng;
    CTR_Mode<AES>::Encryption oPrng;
    uint64_t m_counter_inner;
    uint64_t block_consumed; // number of bytes that have been consumed from the block
    std::array<uint8_t, AES_INTERNAL_BUFFER> block; // buffer of generated randomness
};

void Inner::aesReseedInner() noexcept {
    uint8_t key[AES::DEFAULT_KEYLENGTH];
    oPrng.GenerateBlock(key, sizeof(key));
    uint8_t iv[AES::BLOCKSIZE];
    oPrng.GenerateBlock(iv, sizeof(iv));
    iPrng.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
}

void Inner::aesNextBlock() noexcept {
    if (m_counter_inner >= AES_COUNTER_LIMIT) {
        aesReseedInner();
        m_counter_inner = 0;
    }

    iPrng.GenerateBlock(block.data(), AES_INTERNAL_BUFFER);

    m_counter_inner += AES_PARALLEL_BLOCKS;
}

} // namespace anonymous

namespace sharemind {

AesRandomEngine::AesRandomEngine(const void * seed) {
    if (!supported())
        throw RandomEngine::GeneratorNotSupportedException{};

    m_inner = new Inner{seed};
}

AesRandomEngine::~AesRandomEngine() noexcept
{ delete static_cast<Inner *>(m_inner); }

void AesRandomEngine::fillBytes(void * memptr, size_t size) noexcept {
    if (size <= 0u)
        return;
    assert(memptr);

    Inner & rng = *static_cast<Inner *>(m_inner);
    size_t unconsumedSize = AES_INTERNAL_BUFFER - rng.block_consumed;
    size_t offsetStart = 0;
    size_t offsetEnd = unconsumedSize;

    // Consume full blocks (first might already be partially or entirely consumed).
    while (offsetEnd <= size) {
        memcpy(ptrAdd(memptr, offsetStart), &rng.block[rng.block_consumed], unconsumedSize);
        rng.aesNextBlock();
        rng.block_consumed = 0;
        unconsumedSize = AES_INTERNAL_BUFFER;
        offsetStart = offsetEnd;
        offsetEnd += AES_INTERNAL_BUFFER;
    }

    const size_t remainingSize = size - offsetStart;
    memcpy(ptrAdd(memptr, offsetStart), &rng.block[rng.block_consumed], remainingSize);
    rng.block_consumed += remainingSize;
    assert(rng.block_consumed <= AES_INTERNAL_BUFFER); // the supply may deplete
}

bool AesRandomEngine::supported() noexcept {
    return true;
}

size_t AesRandomEngine::seedSize() noexcept {
    return AES::DEFAULT_KEYLENGTH + AES::BLOCKSIZE;
}

}
