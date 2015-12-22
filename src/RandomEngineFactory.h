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

#ifndef SHAREMIND_LIBRANDOME_RANDOMENGINEFACTORY_H
#define SHAREMIND_LIBRANDOME_RANDOMENGINEFACTORY_H

#include "librandom.h"

#include <cassert>
#include <cstdlib>
#include <sharemind/Exception.h>

namespace sharemind {

/*
 * Here we map SharemindRandomEngineCtorError to exceptions:
 */

SHAREMIND_DEFINE_EXCEPTION(std::exception, RandomCtorError);
SHAREMIND_DEFINE_EXCEPTION(RandomCtorError, RandomCtorSeedError);

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorError, RandomCtorGeneratorNotSupported,
                                     "Unsupported generator");

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorError, RandomCtorOtherError,
                                     "Failed to construct the generator");

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorSeedError, RandomCtorSeedTooShort,
                                     "Provided seed is too short");

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorSeedError, RandomCtorSeedSelfGenerateError,
                                     "Failed to self-generate a seed");

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorSeedError, RandomCtorSeedNotSupported,
                                     "Providing a fixed seed is not supported by this generator");

SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorSeedError, RandomCtorSeedOtherError,
                                     "Failed to seed the generator");

inline
void handleSharemindRandomEngineCtorError(SharemindRandomEngineCtorError& e) {
    switch (e) {
    case SHAREMIND_RANDOM_CTOR_GENERATOR_NOT_SUPPORTED: throw RandomCtorGeneratorNotSupported {};
    case SHAREMIND_RANDOM_CTOR_OUT_OF_MEMORY: throw std::bad_alloc {};
    case SHAREMIND_RANDOM_CTOR_OTHER_ERROR: throw RandomCtorOtherError {};
    case SHAREMIND_RANDOM_CTOR_SEED_TOO_SHORT: throw RandomCtorSeedTooShort {};
    case SHAREMIND_RANDOM_CTOR_SEED_SELF_GENERATE_ERROR: throw RandomCtorSeedSelfGenerateError {};
    case SHAREMIND_RANDOM_CTOR_SEED_NOT_SUPPORTED: throw RandomCtorSeedNotSupported { };
    case SHAREMIND_RANDOM_CTOR_SEED_OTHER_ERROR: throw RandomCtorSeedOtherError {};
    default:
        break;
    }
}

/**
 * \brief Facade for the \a SharemindRandomEngineFactoryFacility
 */

inline
SharemindRandomEngineConf getDefaultRandomEngineConfiguration(
        SharemindRandomEngineFactoryFacility* factory) noexcept
{
    assert(factory);
    return factory->get_default_configuration(factory);
}

inline
SharemindRandomEngine* makeRandomEngine(
        SharemindRandomEngineFactoryFacility* factory,
        SharemindRandomEngineConf conf)
{
    assert(factory);
    auto err = SHAREMIND_RANDOM_CTOR_OK;
    const auto engine = factory->make_random_engine(factory, conf, &err);
    handleSharemindRandomEngineCtorError(err);
    if (!engine)
        throw std::bad_alloc {};

    return engine;
}

inline
SharemindRandomEngine* makeRandomEngineWithSeed(
        SharemindRandomEngineFactoryFacility* factory,
        SharemindRandomEngineConf conf,
        const void* memptr, size_t size)
{
    assert(factory);
    auto err = SHAREMIND_RANDOM_CTOR_OK;
    const auto engine = factory->make_random_engine_with_seed(
                factory, conf, memptr, size, &err);
    handleSharemindRandomEngineCtorError(err);
    if (!engine)
        throw std::bad_alloc {};

    return engine;
}

inline
void freeRandomEngineFactoryFacility(SharemindRandomEngineFactoryFacility* factory)
{
    if (factory)
        factory->free(factory);
}

inline
SharemindRandomEngine* makeRandomEngine(
        SharemindRandomEngineFactoryFacility* factory)
{
    return makeRandomEngine(factory,
                            getDefaultRandomEngineConfiguration(factory));
}

inline
SharemindRandomEngine* makeRandomEngineWithSeed(
        SharemindRandomEngineFactoryFacility* factory,
        const void* memptr, size_t size)
{
    return makeRandomEngineWithSeed(factory,
                                    getDefaultRandomEngineConfiguration(factory),
                                    memptr, size);
}

/**
 * \brief Thin wrapper around SharemindRandomEngineFactoryFacility indicating
 * this object owns the factory facility. The instances is freed on destruction.
 */
class RandomEngineFactoryOwner {
public: /* Methods: */

    explicit RandomEngineFactoryOwner(SharemindRandomEngineFactoryFacility* inner)
        : m_inner (inner)
    { }

    RandomEngineFactoryOwner(const RandomEngineFactoryOwner&) = delete;
    RandomEngineFactoryOwner& operator = (const RandomEngineFactoryOwner&) = delete;
    RandomEngineFactoryOwner(const RandomEngineFactoryOwner&&) = delete;
    RandomEngineFactoryOwner& operator = (const RandomEngineFactoryOwner&&) = delete;

    inline SharemindRandomEngineFactoryFacility* get() const {
        return m_inner;
    }

    ~RandomEngineFactoryOwner() {
        freeRandomEngineFactoryFacility(m_inner);
    }

private: /* Fields: */
    SharemindRandomEngineFactoryFacility* m_inner;
};

/**
 * \brief Construct default implementation of SharemindRandomEngineFactoryFacility
 * \return Well formed SharemindRandomEngineFactoryFacility or nullptr if the configuration ill formed.
 * \throws std::bad_alloc
 */
SharemindRandomEngineFactoryFacility* make_default_random_engine_factory(
        SharemindRandomEngineConf defaultConf);

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOME_RANDOMENGINEFACTORY_H */
