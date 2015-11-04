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

#include "AESRandomEngine.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <openssl/rand.h>
#include <openssl/evp.h>
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

namespace /* anonymouse */ {

inline const EVP_CIPHER* aes_cipher() noexcept {
    return EVP_aes_128_ctr();
}

inline void setErrorFlag(SharemindRandomEngineCtorError* ptr,
                         SharemindRandomEngineCtorError e)
{
    if (ptr != nullptr)
        *ptr = e;
}

extern "C" {
void AESRandomEngine_fill_bytes(SharemindRandomEngine* rng_, void * memptr_, size_t size);
void AESRandomEngine_free(SharemindRandomEngine* rng_);
}

class AESRandomEngine : public SharemindRandomEngine {
public: /* Methods: */

    inline AESRandomEngine(const void* memptr_, SharemindRandomEngineCtorError* e) noexcept
        : SharemindRandomEngine {
              AESRandomEngine_fill_bytes,
              AESRandomEngine_free
          }
        , m_counter_inner (0)
        , m_counter_outer (0)
        , block_consumed (AES_INTERNAL_BUFFER)
    {
        #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
        VALGRIND_MAKE_MEM_DEFINED(this, sizeof(AESRandomEngine));
        #endif

        EVP_CIPHER_CTX_init(&m_ctx_inner);
        EVP_CIPHER_CTX_set_padding(&m_ctx_inner, 0);
        EVP_CIPHER_CTX_init(&m_ctx_outer);
        EVP_CIPHER_CTX_set_padding(&m_ctx_outer, 0);

        uint8_t temp[AES_STATIC_KEY_SIZE];
        memcpy(temp, memptr_, AES_random_engine_seed_size());
        aes_seed(&temp[0], &temp[0] + EVP_CIPHER_key_length(aes_cipher()), e);
    }

    ~AESRandomEngine() {
        EVP_CIPHER_CTX_cleanup(&m_ctx_outer);
        EVP_CIPHER_CTX_cleanup(&m_ctx_inner);
    }

    inline static AESRandomEngine& fromWrapper(SharemindRandomEngine& base) noexcept {
        return static_cast<AESRandomEngine&>(base);
    }

    bool aes_seed(unsigned char* key, unsigned char* iv, SharemindRandomEngineCtorError* e) noexcept;
    bool aes_reseed_inner(SharemindRandomEngineCtorError* e) noexcept;
    void aes_next_block() noexcept;

public: /* Fields: */
    EVP_CIPHER_CTX m_ctx_inner;
    EVP_CIPHER_CTX m_ctx_outer;
    uint64_t m_counter_inner;
    uint64_t m_counter_outer;
    uint64_t block_consumed; // number of bytes that have been consumed from the block
    uint8_t block[AES_INTERNAL_BUFFER]; // buffer of generated randomness
};

inline
bool aes_seed_ctx(EVP_CIPHER_CTX* ctx,
                  unsigned char* key,
                  unsigned char* iv,
                  SharemindRandomEngineCtorError* e) noexcept
{
    if (! EVP_EncryptInit_ex (ctx, aes_cipher(), NULL, key, iv)) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_INTERNAL_ERROR);
        return false;
    }

    return true;
}

bool AESRandomEngine::aes_seed(unsigned char* key,
                               unsigned char* iv,
                               SharemindRandomEngineCtorError* e) noexcept {
    if (! aes_seed_ctx(&m_ctx_outer, key, iv, e)) {
        return false;
    }

    return aes_reseed_inner(e);
}

bool AESRandomEngine::aes_reseed_inner(SharemindRandomEngineCtorError* e) noexcept {
    const auto num_blocks = (AES_random_engine_seed_size() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    const auto plaintext_size = num_blocks * AES_BLOCK_SIZE;
    assert (plaintext_size < AES_STATIC_KEY_SIZE);
    uint8_t plaintext[AES_STATIC_KEY_SIZE];
    uint8_t seed[AES_STATIC_KEY_SIZE];

    memset(&plaintext[0], 0, plaintext_size);
    for (size_t i = 0; i < num_blocks; ++ i) {
        memcpy(&plaintext[i*AES_BLOCK_SIZE], &m_counter_outer, sizeof(m_counter_outer));
        m_counter_outer ++;
    }

    int bytes_written = 0;
    if (! EVP_EncryptUpdate(&m_ctx_outer, &seed[0], &bytes_written, plaintext, plaintext_size)) {
        assert (false && "aes_reseed_inner: Encryption failed.");
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_INTERNAL_ERROR);
        return false;
    }

    assert (bytes_written > 0 && static_cast<decltype(plaintext_size)>(bytes_written) == plaintext_size);

    if (! EVP_EncryptFinal_ex(&m_ctx_outer, &seed[0] + bytes_written, &bytes_written)) {
        assert (false && "aes_reseed_inner: EncryptFinal failed.");
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_INTERNAL_ERROR);
        return false;
    }

    assert(bytes_written == 0);
    const auto keyLen = EVP_CIPHER_key_length(aes_cipher());
    return aes_seed_ctx(&m_ctx_inner, &seed[0], &seed[0] + keyLen, e);
}

void AESRandomEngine::aes_next_block() noexcept {
    if (m_counter_inner >= AES_COUNTER_LIMIT) {
        (void) aes_reseed_inner(nullptr);
        m_counter_inner = 0;
    }

    unsigned char plaintext[AES_INTERNAL_BUFFER];
    memset(&plaintext[0], 0, sizeof (plaintext));
    for (size_t i = 0; i < AES_PARALLEL_BLOCKS; ++ i) {
        memcpy(&plaintext[i*AES_BLOCK_SIZE], &m_counter_inner, sizeof(m_counter_inner));
        m_counter_inner ++;
    }

    int bytes_written = 0;
    if (! EVP_EncryptUpdate(&m_ctx_inner, &block[0], &bytes_written, plaintext, AES_INTERNAL_BUFFER)) {
        assert (false && "Encryption failed.");
        return;
    }

    assert (bytes_written == AES_INTERNAL_BUFFER);
    if (! EVP_EncryptFinal_ex(&m_ctx_inner, &block[0] + bytes_written, &bytes_written)) {
        assert (false && "EncryptFinal failed.");
        return;
    }

    assert(bytes_written == 0);
}

extern "C"
void AESRandomEngine_fill_bytes(SharemindRandomEngine* rng_, void * memptr_, size_t size)
{
    assert (rng_ != nullptr);
    assert (memptr_ != nullptr);

    auto& rng = AESRandomEngine::fromWrapper(*rng_);
    uint8_t* memptr = static_cast<uint8_t*>(memptr_);
    size_t unconsumedSize = AES_INTERNAL_BUFFER - rng.block_consumed;
    size_t offsetStart = 0;
    size_t offsetEnd = unconsumedSize;

    // Consume full blocks (first might already be partially or entirely consumed).
    while (offsetEnd <= size) {
        memcpy(memptr + offsetStart, &rng.block[rng.block_consumed], unconsumedSize);
        rng.aes_next_block();
        rng.block_consumed = 0;
        unconsumedSize = AES_INTERNAL_BUFFER;
        offsetStart = offsetEnd;
        offsetEnd += AES_INTERNAL_BUFFER;
    }

    const size_t remainingSize = size - offsetStart;
    memcpy(memptr + offsetStart, &rng.block[rng.block_consumed], remainingSize);
    rng.block_consumed += remainingSize;
    assert (rng.block_consumed <= AES_INTERNAL_BUFFER); // the supply may deplete
}

extern "C"
void AESRandomEngine_free(SharemindRandomEngine* rng_) {
    assert (rng_ != nullptr);
    delete &AESRandomEngine::fromWrapper(*rng_);
}


} // namespace anonymous

namespace sharemind {

size_t AES_random_engine_seed_size() noexcept {
    if (aes_cipher() == nullptr)
        return 0;

    return EVP_CIPHER_key_length(aes_cipher()) + EVP_CIPHER_iv_length(aes_cipher());
}

SharemindRandomEngine* make_AES_random_engine(const void* memptr_, SharemindRandomEngineCtorError* e) {

    // Make sure that the cipher we use is defined.
    if (aes_cipher() == nullptr) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_NOT_SUPPORTED);
        return nullptr;
    }

    // Make sure that our seed, aligned to AES_BLOCK_SIZE, fits into AES_STATIC_KEY_SIZE.
    const auto num_blocks = (AES_random_engine_seed_size() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    const auto seed_size = num_blocks * AES_BLOCK_SIZE;
    if (seed_size > AES_STATIC_KEY_SIZE) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_INTERNAL_ERROR);
        return nullptr;
    }

    return new AESRandomEngine {memptr_, e};
}

}
