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

#include "AesRandomEngine.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <openssl/evp.h>
#include <sharemind/abort.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#define AES_BLOCK_SIZE 16

// The number of AES blocks to generate at a time.
// Because we are working in CTR mode we can generate those in parallel.
// Significant speedup over doing it a single block at a time.
#define AES_PARALLEL_BLOCKS 8

#define AES_INTERNAL_BUFFER (AES_PARALLEL_BLOCKS*AES_BLOCK_SIZE)

// The number of AES blocks after which we reseed the inner generator.
// This number has been selected to achieve less than 2^{-80} advantage.
#define AES_COUNTER_LIMIT (1u << 24u)

// This is a workaround for not knowing the
// size of the key during compile time in order
// to avoid using dynamic memory allocation.
#define AES_STATIC_KEY_SIZE 128

using namespace sharemind;

namespace /* anonymous */ {

inline const EVP_CIPHER* aes_cipher() noexcept {
    return EVP_aes_128_ctr();
}

struct Inner {

    inline Inner(const void * memptr_) noexcept
        : m_counter_inner (0)
        , m_counter_outer (0)
        , block_consumed (AES_INTERNAL_BUFFER)
    {
        #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
        VALGRIND_MAKE_MEM_DEFINED(this, sizeof(Inner));
        #endif

        EVP_CIPHER_CTX_init(&m_ctx_inner);
        EVP_CIPHER_CTX_set_padding(&m_ctx_inner, 0);
        EVP_CIPHER_CTX_init(&m_ctx_outer);
        EVP_CIPHER_CTX_set_padding(&m_ctx_outer, 0);

        uint8_t temp[AES_STATIC_KEY_SIZE];
        memcpy(temp, memptr_, AesRandomEngine::seedSize());
        aes_seed(&temp[0], &temp[0] + EVP_CIPHER_key_length(aes_cipher()));
    }

    ~Inner() noexcept {
        EVP_CIPHER_CTX_cleanup(&m_ctx_outer);
        EVP_CIPHER_CTX_cleanup(&m_ctx_inner);
    }

    void aes_seed(unsigned char * key, unsigned char * iv) noexcept;
    void aes_reseed_inner() noexcept;
    void aes_next_block() noexcept;

    EVP_CIPHER_CTX m_ctx_inner;
    EVP_CIPHER_CTX m_ctx_outer;
    uint64_t m_counter_inner;
    uint64_t m_counter_outer;
    uint64_t block_consumed; // number of bytes that have been consumed from the block
    uint8_t block[AES_INTERNAL_BUFFER]; // buffer of generated randomness
};

inline void aes_seed_ctx(EVP_CIPHER_CTX * ctx,
                         unsigned char * key,
                         unsigned char * iv) noexcept
{
    if (!EVP_EncryptInit_ex (ctx, aes_cipher(), NULL, key, iv))
        throw AesRandomEngine::InitException{};
}

void Inner::aes_seed(unsigned char * key, unsigned char * iv) noexcept {
    aes_seed_ctx(&m_ctx_outer, key, iv);
    aes_reseed_inner();
}

void Inner::aes_reseed_inner() noexcept {
    const auto num_blocks = (AesRandomEngine::seedSize() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    const auto plaintext_size = num_blocks * AES_BLOCK_SIZE;
    assert(plaintext_size < AES_STATIC_KEY_SIZE);
    uint8_t plaintext[AES_STATIC_KEY_SIZE];
    uint8_t seed[AES_STATIC_KEY_SIZE];

    memset(&plaintext[0], 0, plaintext_size);
    for (size_t i = 0; i < num_blocks; ++ i) {
        memcpy(&plaintext[i*AES_BLOCK_SIZE], &m_counter_outer, sizeof(m_counter_outer));
        m_counter_outer ++;
    }

    int bytes_written = 0;
    if (!EVP_EncryptUpdate(&m_ctx_outer, &seed[0], &bytes_written, plaintext, plaintext_size))
        SHAREMIND_ABORT("aes_reseed_inner: Encryption failed!");

    assert(bytes_written > 0
           && static_cast<decltype(plaintext_size)>(bytes_written)
              == plaintext_size);

    if (!EVP_EncryptFinal_ex(&m_ctx_outer, &seed[0] + bytes_written, &bytes_written))
        SHAREMIND_ABORT("aes_reseed_inner: EncryptFinal failed!");

    assert(bytes_written == 0);
    const auto keyLen = EVP_CIPHER_key_length(aes_cipher());
    aes_seed_ctx(&m_ctx_inner, &seed[0], &seed[0] + keyLen);
}

void Inner::aes_next_block() noexcept {
    if (m_counter_inner >= AES_COUNTER_LIMIT) {
        aes_reseed_inner();
        m_counter_inner = 0;
    }

    unsigned char plaintext[AES_INTERNAL_BUFFER];
    memset(&plaintext[0], 0, sizeof (plaintext));
    for (size_t i = 0; i < AES_PARALLEL_BLOCKS; ++ i) {
        memcpy(&plaintext[i*AES_BLOCK_SIZE], &m_counter_inner, sizeof(m_counter_inner));
        m_counter_inner ++;
    }

    int bytes_written = 0;
    if (!EVP_EncryptUpdate(&m_ctx_inner, &block[0], &bytes_written, plaintext, AES_INTERNAL_BUFFER))
        SHAREMIND_ABORT("Encryption failed.");

    assert(bytes_written == AES_INTERNAL_BUFFER);
    if (!EVP_EncryptFinal_ex(&m_ctx_inner, &block[0] + bytes_written, &bytes_written))
        SHAREMIND_ABORT("EncryptFinal failed.");

    assert(bytes_written == 0);
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
        rng.aes_next_block();
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
    if (!aes_cipher())
        return false;

    // Make sure that our seed, aligned to AES_BLOCK_SIZE, fits into AES_STATIC_KEY_SIZE.
    auto const num_blocks = (AesRandomEngine::seedSize() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    auto const seed_size = num_blocks * AES_BLOCK_SIZE;
    return seed_size <= AES_STATIC_KEY_SIZE;
}

size_t AesRandomEngine::seedSize() noexcept {
    if (!aes_cipher())
        return 0;

    return EVP_CIPHER_key_length(aes_cipher()) + EVP_CIPHER_iv_length(aes_cipher());
}

}
