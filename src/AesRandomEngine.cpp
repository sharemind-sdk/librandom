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
#include <sharemind/PotentiallyVoidTypeInfo.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>


namespace sharemind {
namespace {

constexpr static std::size_t const AES_BLOCK_SIZE = 16u;

// The number of AES blocks to generate at a time.
// Because we are working in CTR mode we can generate those in parallel.
// Significant speedup over doing it a single block at a time.
constexpr static std::size_t const AES_PARALLEL_BLOCKS = 8u;


constexpr static std::size_t const AES_INTERNAL_BUFFER =
        AES_PARALLEL_BLOCKS * AES_BLOCK_SIZE;

// The number of AES blocks after which we reseed the inner generator.
// This number has been selected to achieve less than 2^{-80} advantage.
constexpr static std::size_t const AES_COUNTER_LIMIT = (1u << 24u);

struct Inner {

    inline Inner(void const * const memptr_) noexcept
        : m_counterInner(0)
        , m_blockConsumed(AES_INTERNAL_BUFFER)
    {
        auto const key = static_cast<CryptoPP::byte const *>(memptr_);
        m_oPrng.SetKeyWithIV(key,
                             CryptoPP::AES::DEFAULT_KEYLENGTH,
                             key + CryptoPP::AES::DEFAULT_KEYLENGTH,
                             CryptoPP::AES::BLOCKSIZE);
        aesReseedInner();
    }

    ~Inner() noexcept = default;

    void aesReseedInner() noexcept;
    void aesNextBlock() noexcept;

    CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_iPrng;
    CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_oPrng;
    std::uint64_t m_counterInner;

    /// number of bytes that have been consumed from the block:
    std::uint64_t m_blockConsumed;

    /// buffer of generated randomness
    std::array<std::uint8_t, AES_INTERNAL_BUFFER> m_block;
};

void Inner::aesReseedInner() noexcept {
    std::uint8_t key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    m_oPrng.GenerateBlock(key, sizeof(key));
    std::uint8_t iv[CryptoPP::AES::BLOCKSIZE];
    m_oPrng.GenerateBlock(iv, sizeof(iv));
    m_iPrng.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
}

void Inner::aesNextBlock() noexcept {
    if (m_counterInner >= AES_COUNTER_LIMIT) {
        aesReseedInner();
        m_counterInner = 0u;
    }

    m_iPrng.GenerateBlock(m_block.data(), AES_INTERNAL_BUFFER);

    m_counterInner += AES_PARALLEL_BLOCKS;
}

} // namespace anonymous

AesRandomEngine::AesRandomEngine(const void * seed) {
    if (!supported())
        throw RandomEngine::GeneratorNotSupportedException();

    m_inner = new Inner(seed);
}

AesRandomEngine::~AesRandomEngine() noexcept
{ delete static_cast<Inner *>(m_inner); }

void AesRandomEngine::fillBytes(void * memptr, std::size_t size) noexcept {
    if (size <= 0u)
        return;
    assert(memptr);

    Inner & rng = *static_cast<Inner *>(m_inner);
    std::size_t unconsumedSize = AES_INTERNAL_BUFFER - rng.m_blockConsumed;
    std::size_t offsetStart = 0u;
    std::size_t offsetEnd = unconsumedSize;

    /* Consume full blocks (first might already be partially or entirely
       consumed). */
    while (offsetEnd <= size) {
        std::memcpy(ptrAdd(memptr, offsetStart),
                    &rng.m_block[rng.m_blockConsumed],
                    unconsumedSize);
        rng.aesNextBlock();
        rng.m_blockConsumed = 0u;
        unconsumedSize = AES_INTERNAL_BUFFER;
        offsetStart = offsetEnd;
        offsetEnd += AES_INTERNAL_BUFFER;
    }

    std::size_t const remainingSize = size - offsetStart;
    std::memcpy(ptrAdd(memptr, offsetStart),
                &rng.m_block[rng.m_blockConsumed],
                remainingSize);
    rng.m_blockConsumed += remainingSize;

    // the supply may deplete:
    assert(rng.m_blockConsumed <= AES_INTERNAL_BUFFER);
}

bool AesRandomEngine::supported() noexcept { return true; }

size_t AesRandomEngine::seedSize() noexcept
{ return CryptoPP::AES::DEFAULT_KEYLENGTH + CryptoPP::AES::BLOCKSIZE; }

}
