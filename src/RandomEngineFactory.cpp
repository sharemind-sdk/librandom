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

#include "AesRandomEngine.h"
#include "ChaCha20RandomEngine.h"
#include "CryptographicRandom.h"
#include "NullRandomEngine.h"
#include "RandomBufferAgent.h"
#include "RandomEngine.h"
#include "Snow2RandomEngine.h"

#include <exception>
#include <memory>
#include <mutex>
#include <new>
#ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#define SEED_TEMP_BUFFER_SIZE 256

namespace sharemind {

//RandomEngine * RandomEngineFactory::createRandomEngine(
//        RandomEngineFactory::Configuration const & conf)
//{
//    auto const seedSize = getSeedSize(conf.coreEngine);
//    if (seedSize > SEED_TEMP_BUFFER_SIZE)
//        throw RandomCtorSeedSelfGenerateError{};

//    unsigned char tempBuffer[SEED_TEMP_BUFFER_SIZE];

//    #ifdef SHAREMIND_LIBRANDOM_HAVE_VALGRIND
//    VALGRIND_MAKE_MEM_DEFINED(tempBuffer, sizeof(tempBuffer));
//    #endif

//    cryptographicURandom(tempBuffer, seedSize);

//    return createRandomEngineWithSeed(conf, tempBuffer, seedSize);
//}

size_t RandomEngineFactory::getSeedSize(SharemindCoreRandomEngineKind kind) noexcept {
    switch (kind) {
    case SHAREMIND_RANDOM_SNOW2:    return Snow2RandomEngine::SeedSize;
    case SHAREMIND_RANDOM_CHACHA20: return ChaCha20RandomEngine::SeedSize;
    case SHAREMIND_RANDOM_AES:      return AesRandomEngine::seedSize();
    default:                        return 0u;
    }
}


RandomEngine * RandomEngineFactory::createRandomEngineWithSeed(
        Configuration const & conf,
        void const * seedData,
        size_t seedSize)
{
    // Verify seed:
    auto const needSeedSize = getSeedSize(conf.coreEngine);
    if (needSeedSize > seedSize)
        throw RandomCtorSeedTooShort{};

    // Construct core engine:
    RandomEngine * coreEngine;
    switch (conf.coreEngine) {
        case SHAREMIND_RANDOM_NULL:
            if (seedSize > 0u)
                throw RandomCtorSeedNotSupported{};
            return &NullRandomEngine::instance();
        case SHAREMIND_RANDOM_SNOW2:
            coreEngine = new Snow2RandomEngine(seedData);
            break;
        case SHAREMIND_RANDOM_CHACHA20:
            coreEngine = new ChaCha20RandomEngine(seedData);
            break;
        case SHAREMIND_RANDOM_AES:
            coreEngine = new AesRandomEngine(seedData);
            break;
        default:
            throw RandomCtorGeneratorNotSupported{};
    }

    // Add buffering if need be:
    switch (conf.bufferMode) {
    case SHAREMIND_RANDOM_BUFFERING_NONE:
        return coreEngine;
    case SHAREMIND_RANDOM_BUFFERING_THREAD:
            return new RandomBufferAgent{
                std::unique_ptr<RandomEngine>{coreEngine},
                conf.bufferSize};
    default:
        throw RandomCtorOtherError{};
    }
}

} // namespace sharemind
