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

#include "RandomEngineFactory.h"

#include "AESRandomEngine.h"
#include "ChaCha20RandomEngine.h"
#include "NullRandomEngine.h"
#include "OpenSSLRandomEngine.h"
#include "RandomBufferAgent.h"
#include "RandomEngine.h"
#include "SNOW2RandomEngine.h"

#include <exception>
#include <new>
#include <openssl/rand.h>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#define SEED_TEMP_BUFFER_SIZE 256

using namespace sharemind;

namespace /* anonymous */ {

extern "C" {

SharemindRandomEngineConf RandomEngineFactoryImpl_get_default_configuration(
        const SharemindRandomEngineFactoryFacility* facility);

SharemindRandomEngine* RandomEngineFactoryImpl_make_random_engine(
        const SharemindRandomEngineFactoryFacility* facility,
        SharemindRandomEngineConf conf,
        SharemindRandomEngineCtorError* e);

SharemindRandomEngine* RandomEngineFactoryImpl_make_random_engine_with_seed(
        const SharemindRandomEngineFactoryFacility* facility,
        SharemindRandomEngineConf conf,
        const void* memptr,
        size_t size,
        SharemindRandomEngineCtorError* e);

void RandomEngineFactoryImpl_free(SharemindRandomEngineFactoryFacility* facility);

}

class RandomEngineFactoryImpl: public SharemindRandomEngineFactoryFacility {
public: /* Methods: */

    inline RandomEngineFactoryImpl(SharemindRandomEngineConf conf)
        : SharemindRandomEngineFactoryFacility {
            RandomEngineFactoryImpl_get_default_configuration,
            RandomEngineFactoryImpl_make_random_engine,
            RandomEngineFactoryImpl_make_random_engine_with_seed,
            RandomEngineFactoryImpl_free
        }
        , m_conf (conf)
    { }

    inline static const RandomEngineFactoryImpl& fromWrapper(const SharemindRandomEngineFactoryFacility& base) noexcept {
        return static_cast<const RandomEngineFactoryImpl&>(base);
    }

public: /* Fields: */
    const SharemindRandomEngineConf m_conf;
};

inline void setErrorFlag(SharemindRandomEngineCtorError* ptr,
                         SharemindRandomEngineCtorError e)
{
    if (ptr != nullptr)
        *ptr = e;
}

inline void seedHandleExceptions(SharemindRandomEngineCtorError* e) noexcept {
    try {
        if (const auto eptr = std::current_exception()) {
            std::rethrow_exception(eptr);
        }
    }
    catch (const std::bad_alloc &) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_OUT_OF_MEMORY);
    }
    catch (...) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_INTERNAL_ERROR);
    }
}

inline
SharemindRandomEngine* makeThreadBufferedEngine(SharemindRandomEngine* coreEngine,
                                                size_t bufferSize,
                                                SharemindRandomEngineCtorError* e) noexcept
{
    assert (coreEngine != nullptr);

    try {
        return make_thread_buffered_random_engine(RandomEngine {coreEngine}, bufferSize);
    }
    catch (...) {
        seedHandleExceptions(e);
        return nullptr;
    }
}

inline size_t getSeedSize(SharemindCoreRandomEngineKind kind) noexcept {
    switch (kind) {
    case SHAREMIND_RANDOM_SNOW2: return SNOW2_random_engine_seed_size();
    case SHAREMIND_RANDOM_CHACHA20: return ChaCha20_random_engine_seed_size();
    case SHAREMIND_RANDOM_AES: return AES_random_engine_seed_size();
    default:
        return 0;
    }
}

extern "C"
SharemindRandomEngineConf RandomEngineFactoryImpl_get_default_configuration(
        const SharemindRandomEngineFactoryFacility* facility)
{
    assert (facility != nullptr);
    return RandomEngineFactoryImpl::fromWrapper(*facility).m_conf;
}

extern "C"
SharemindRandomEngine* RandomEngineFactoryImpl_make_random_engine(
        const SharemindRandomEngineFactoryFacility* facility,
        SharemindRandomEngineConf conf,
        SharemindRandomEngineCtorError* e)
{
    assert (facility != nullptr);

    const auto seedSize = getSeedSize(conf.core_engine);
    if (seedSize > SEED_TEMP_BUFFER_SIZE) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_SELF_GENERATE_ERROR);
        return nullptr;
    }

    unsigned char tempBuffer[SEED_TEMP_BUFFER_SIZE];

    #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
    VALGRIND_MAKE_MEM_DEFINED(tempBuffer, sizeof(tempBuffer));
    #endif

    const auto result = RAND_bytes(tempBuffer, seedSize);
    if (result != 1) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_SELF_GENERATE_ERROR);
        return nullptr;
    }

    return RandomEngineFactoryImpl_make_random_engine_with_seed(
                facility, conf, tempBuffer, seedSize, e);
}

extern "C"
SharemindRandomEngine* RandomEngineFactoryImpl_make_random_engine_with_seed(
        const SharemindRandomEngineFactoryFacility* facility,
        SharemindRandomEngineConf conf,
        const void* memptr,
        size_t size,
        SharemindRandomEngineCtorError* e)
{
    assert (facility != nullptr);
    assert (memptr != nullptr);

    // Verify seed:

    const auto seedSize = getSeedSize(conf.core_engine);
    if (seedSize > size) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_INSUFFICIENT_ENTROPY);
        return nullptr;
    }

    // Construct core engine:

    SharemindRandomEngine* coreEngine = nullptr;

    try {
        switch (conf.core_engine) {
        case SHAREMIND_RANDOM_SNOW2:
            coreEngine = make_SNOW2_random_engine(memptr);
            break;
        case SHAREMIND_RANDOM_CHACHA20:
            coreEngine = make_ChaCha20_random_engine(memptr);
            break;
        case SHAREMIND_RANDOM_AES:
            coreEngine = make_AES_random_engine(memptr, e);
            if (coreEngine == nullptr)
                return nullptr;
            break;
        case SHAREMIND_RANDOM_NULL:
            coreEngine = make_null_random_engine();
            break;
        case SHAREMIND_RANDOM_OPENSSL:
            if (size > 0) {
                setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_NOT_SUPPORTED);
                return nullptr;
            }

            coreEngine = make_OpenSSL_random_engine();
            break;
        }
    }
    catch (...) {
        seedHandleExceptions(e);
        return nullptr;
    }

    if (coreEngine == nullptr) {
        setErrorFlag(e, SHAREMIND_RANDOM_CTOR_SEED_INTERNAL_ERROR);
        return nullptr;
    }

    // Add buffering if need be:
    switch (conf.buffer_mode) {
    case SHAREMIND_RANDOM_BUFFERING_NONE:
        return coreEngine;
    case SHAREMIND_RANDOM_BUFFERING_THREAD:
        return makeThreadBufferedEngine(coreEngine, conf.buffer_size, e);
    default:
        break;
    }

    return nullptr;
}

extern "C"
void RandomEngineFactoryImpl_free(SharemindRandomEngineFactoryFacility* facility) {
    if (facility != nullptr) {
        delete &RandomEngineFactoryImpl::fromWrapper(*facility);
    }
}


} // namespace anonymous

namespace sharemind {

SharemindRandomEngineFactoryFacility* make_default_random_engine_factory(
        SharemindRandomEngineConf defaultConf)
{
    return new RandomEngineFactoryImpl {defaultConf};
}

} // namespace sharemind
