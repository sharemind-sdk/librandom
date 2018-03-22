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


namespace sharemind {

SHAREMIND_DEFINE_EXCEPTION_NOINLINE(sharemind::Exception,
                                    RandomEngineFactory::,
                                    Exception);
SHAREMIND_DEFINE_EXCEPTION_NOINLINE(Exception,
                                    RandomEngineFactory::,
                                    RandomCtorSeedException);
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(Exception,
                                              RandomEngineFactory::,
                                              RandomCtorGeneratorNotSupported,
                                              "Unsupported generator");
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        Exception,
        RandomEngineFactory::,
        RandomCtorOtherError,
        "Failed to construct the generator");
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(RandomCtorSeedException,
                                              RandomEngineFactory::,
                                              RandomCtorSeedTooShort,
                                              "Provided seed is too short");
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(RandomCtorSeedException,
                                              RandomEngineFactory::,
                                              RandomCtorSeedSelfGenerateError,
                                              "Failed to self-generate a seed");
SHAREMIND_DEFINE_EXCEPTION_CONST_MSG_NOINLINE(
        RandomCtorSeedException,
        RandomEngineFactory::,
        RandomCtorSeedNotSupported,
        "Providing a fixed seed is not supported by this generator");

size_t RandomEngineFactory::getSeedSize(SharemindCoreRandomEngineKind kind) noexcept {
    switch (kind) {
    case SHAREMIND_RANDOM_SNOW2:    return Snow2RandomEngine::SeedSize;
    case SHAREMIND_RANDOM_CHACHA20: return ChaCha20RandomEngine::SeedSize;
    case SHAREMIND_RANDOM_AES:      return AesRandomEngine::seedSize();
    default:                        return 0u;
    }
}


std::shared_ptr<RandomEngine> RandomEngineFactory::createRandomEngineWithSeed(
        Configuration const & conf,
        void const * seedData,
        size_t seedSize)
{
    // Verify seed:
    auto const needSeedSize = getSeedSize(conf.coreEngine);
    if (needSeedSize > seedSize)
        throw RandomCtorSeedTooShort{};

    // Construct core engine:
    std::shared_ptr<RandomEngine> coreEngine;
    switch (conf.coreEngine) {
        case SHAREMIND_RANDOM_NULL:
            if (seedSize > 0u)
                throw RandomCtorSeedNotSupported{};
            return std::shared_ptr<RandomEngine>(&NullRandomEngine::instance(),
                                                 [](RandomEngine * const){});
        case SHAREMIND_RANDOM_SNOW2:
            coreEngine = std::make_shared<Snow2RandomEngine>(seedData);
            break;
        case SHAREMIND_RANDOM_CHACHA20:
            coreEngine = std::make_shared<ChaCha20RandomEngine>(seedData);
            break;
        case SHAREMIND_RANDOM_AES:
            coreEngine = std::make_shared<AesRandomEngine>(seedData);
            break;
        default:
            throw RandomCtorGeneratorNotSupported{};
    }

    // Add buffering if need be:
    switch (conf.bufferMode) {
    case SHAREMIND_RANDOM_BUFFERING_NONE:
        return coreEngine;
    case SHAREMIND_RANDOM_BUFFERING_THREAD:
            return std::make_shared<RandomBufferAgent>(coreEngine,
                                                       conf.bufferSize);
    default:
        throw RandomCtorOtherError{};
    }
}

} // namespace sharemind
