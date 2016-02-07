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
#include <limits>
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
        aesSeed(&temp[0], &temp[0] + EVP_CIPHER_key_length(aes_cipher()));
    }

    ~Inner() noexcept {
        EVP_CIPHER_CTX_cleanup(&m_ctx_outer);
        EVP_CIPHER_CTX_cleanup(&m_ctx_inner);
    }

    void aesSeed(unsigned char * key, unsigned char * iv) noexcept;
    void aesReseedInner() noexcept;
    void aesNextBlock() noexcept;

    EVP_CIPHER_CTX m_ctx_inner;
    EVP_CIPHER_CTX m_ctx_outer;
    uint64_t m_counter_inner;
    uint64_t block_consumed; // number of bytes that have been consumed from the block
    uint8_t block[AES_INTERNAL_BUFFER]; // buffer of generated randomness
};

inline void aesSeedCtx(EVP_CIPHER_CTX * ctx,
                         unsigned char * key,
                         unsigned char * iv) noexcept
{
    if (!EVP_EncryptInit_ex (ctx, aes_cipher(), NULL, key, iv))
        throw AesRandomEngine::InitException{};
}

void Inner::aesSeed(unsigned char * key, unsigned char * iv) noexcept {
    aesSeedCtx(&m_ctx_outer, key, iv);
    aesReseedInner();
}

void Inner::aesReseedInner() noexcept {
    const auto numBlocks = (AesRandomEngine::seedSize() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    const auto plaintextSize = numBlocks * AES_BLOCK_SIZE;
    assert(plaintextSize < AES_STATIC_KEY_SIZE);
    uint8_t plaintext[AES_STATIC_KEY_SIZE] = {};
    uint8_t seed[AES_STATIC_KEY_SIZE] = {};

    int bytesWritten = 0;
    assert(plaintextSize <= std::numeric_limits<int>::max());
    if (!EVP_EncryptUpdate(&m_ctx_outer,
                           &seed[0],
                           &bytesWritten,
                           plaintext,
                           static_cast<int>(plaintextSize)))
        SHAREMIND_ABORT("aesReseedInner: Encryption failed!");

    assert(bytesWritten > 0
           && static_cast<decltype(plaintextSize)>(bytesWritten)
              == plaintextSize);

    if (!EVP_EncryptFinal_ex(&m_ctx_outer, &seed[0] + bytesWritten, &bytesWritten))
        SHAREMIND_ABORT("aesReseedInner: EncryptFinal failed!");

    assert(bytesWritten == 0);
    const auto keyLen = EVP_CIPHER_key_length(aes_cipher());
    aesSeedCtx(&m_ctx_inner, &seed[0], &seed[0] + keyLen);
}

void Inner::aesNextBlock() noexcept {
    if (m_counter_inner >= AES_COUNTER_LIMIT) {
        aesReseedInner();
        m_counter_inner = 0;
    }

    // We are using a temporary zero buffer because OpenSSL documentation does
    // not explicitly state that in-place encrypts are allowed.
    unsigned char plaintext[AES_INTERNAL_BUFFER] = { };

    int bytesWritten = 0;
    if (!EVP_EncryptUpdate(&m_ctx_inner, &block[0], &bytesWritten, plaintext, AES_INTERNAL_BUFFER))
        SHAREMIND_ABORT("Encryption failed.");

    assert(bytesWritten == AES_INTERNAL_BUFFER);
    if (!EVP_EncryptFinal_ex(&m_ctx_inner, &block[0] + bytesWritten, &bytesWritten))
        SHAREMIND_ABORT("EncryptFinal failed.");

    m_counter_inner += AES_PARALLEL_BLOCKS;
    assert(bytesWritten == 0);
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
    if (!aes_cipher())
        return false;

    // Make sure that our seed, aligned to AES_BLOCK_SIZE, fits into AES_STATIC_KEY_SIZE.
    auto const numBlocks = (AesRandomEngine::seedSize() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    auto const seedSize = numBlocks * AES_BLOCK_SIZE;
    return seedSize <= AES_STATIC_KEY_SIZE;
}

size_t AesRandomEngine::seedSize() noexcept {
    if (!aes_cipher())
        return 0;

    return static_cast<size_t>(EVP_CIPHER_key_length(aes_cipher()))
           + static_cast<size_t>(EVP_CIPHER_iv_length(aes_cipher()));
}

}
