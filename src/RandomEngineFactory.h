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
#include <stdexcept>

namespace sharemind {

/**
 * \brief Facade for the \a SharemindRandomEngineFactoryFacility
 */

inline
SharemindRandomEngineConf getDefaultRandomEngineConfiguration(
        SharemindRandomEngineFactoryFacility* factory) noexcept
{
    assert (factory != nullptr);
    return factory->get_default_configuration(factory);
}

inline
SharemindRandomEngine* makeRandomEngine(
        SharemindRandomEngineFactoryFacility* factory,
        SharemindRandomEngineConf conf)
{
    assert (factory != nullptr);
    auto err = SHAREMIND_RANDOM_CTOR_OK;
    const auto engine = factory->make_random_engine(factory, conf, &err);
    if (err != SHAREMIND_RANDOM_CTOR_OK || engine == nullptr) {
        // TODO: throw a proper exception!
        throw std::runtime_error("makeRandomEngine failed");
    }

    return engine;
}

inline
SharemindRandomEngine* makeRandomEngineWithSeed(
        SharemindRandomEngineFactoryFacility* factory,
        SharemindRandomEngineConf conf,
        const void* memptr, size_t size)
{
    assert (factory != nullptr);
    auto err = SHAREMIND_RANDOM_CTOR_OK;
    const auto engine = factory->make_random_engine_with_seed(factory, conf, memptr, size, &err);
    if (err != SHAREMIND_RANDOM_CTOR_OK || engine == nullptr) {
        // TODO: throw a proper exception!
        throw std::runtime_error("makeRandomEngineWithSeed failed");
    }

    return engine;
}

inline
void freeRandomEngineFactoryFacility(SharemindRandomEngineFactoryFacility* factory)
{
    if (factory != nullptr)
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
