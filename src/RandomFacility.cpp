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

#include "RandomFacility.h"

#include <cassert>
#include <memory>
#include <sharemind/visibility.h>
#include "RandomEngine.h"


namespace sharemind {

namespace {

extern "C" void SharemindRandomEngine_fillBytes(SharemindRandomEngine * rng,
                                                void * memptr,
                                                size_t size) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

} // anonymous namespace

struct ScopedEngine: SharemindRandomEngine {

    ScopedEngine(RandomEngine * const engine)
        : SharemindRandomEngine{&SharemindRandomEngine_fillBytes}
        , m_engine{(assert(engine), engine)}
    {}

    void fillBytes(void * const buffer, size_t const bufferSize) noexcept
    { m_engine->fillBytes(buffer, bufferSize); }

    std::unique_ptr<RandomEngine> m_engine;

};

namespace {

inline ScopedEngine & fromWrapper(SharemindRandomEngine & base) noexcept
{ return static_cast<ScopedEngine &>(base); }

extern "C" void SharemindRandomEngine_fillBytes(SharemindRandomEngine * rng,
                                                void * memptr,
                                                size_t size) noexcept
{ fromWrapper((assert(rng), *rng)).fillBytes(memptr, size); }

inline RandomFacility & fromWrapper(SharemindRandomFacility & base) noexcept
{ return static_cast<RandomFacility &>(base); }

inline RandomFacility const & fromWrapper(SharemindRandomFacility const & base)
        noexcept
{ return static_cast<RandomFacility const &>(base); }

extern "C"
SharemindRandomEngineConf const *
SharemindRandomFacility_defaultFactoryConfiguration(
        SharemindRandomFacility const * facility) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
SharemindRandomEngineConf const *
SharemindRandomFacility_defaultFactoryConfiguration(
        SharemindRandomFacility const * facility) noexcept
{
    assert(facility);
    return &fromWrapper(*facility).defaultFactoryConfiguration();
}

inline void handleException(SharemindRandomEngineCtorError * const e) {
    if (!e)
        return;
    using REF = RandomEngineFactory;
    try {
        throw;
    } catch (REF::RandomCtorGeneratorNotSupported const &) {
        *e = SHAREMIND_RANDOM_GENERATOR_NOT_SUPPORTED;
    } catch (std::bad_alloc const &) {
        *e = SHAREMIND_RANDOM_OUT_OF_MEMORY;
    } catch (REF::RandomCtorOtherError const &) {
        *e = SHAREMIND_RANDOM_GENERAL_ERROR;
    } catch (REF::RandomCtorSeedTooShort const &) {
        *e = SHAREMIND_RANDOM_SEED_TOO_SHORT;
    } catch (REF::RandomCtorSeedSelfGenerateError const &) {
        *e = SHAREMIND_RANDOM_SEED_GENERATION_ERROR;
    } catch (REF::RandomCtorSeedNotSupported const &) {
        *e = SHAREMIND_RANDOM_SEED_NOT_SUPPORTED;
    } catch (RandomEngine::Exception const &) {
        *e = SHAREMIND_RANDOM_GENERAL_ERROR;
    } catch (...) {
        *e = SHAREMIND_RANDOM_UNKNOWN_ERROR;
    }
}

#define SHAREMIND_RANDOMFACILITY_TRY(...) \
    try { __VA_ARGS__ } catch (...) { \
        handleException(errorPtr); \
        return nullptr; \
    }


extern "C"
SharemindRandomEngine * SharemindRandomFacility_createRandomEngine(
        SharemindRandomFacility * facility,
        SharemindRandomEngineConf const * conf,
        SharemindRandomEngineCtorError * errorPtr) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
SharemindRandomEngine * SharemindRandomFacility_createRandomEngine(
        SharemindRandomFacility * facility,
        SharemindRandomEngineConf const * conf,
        SharemindRandomEngineCtorError * errorPtr) noexcept
{
    assert(facility);
    SHAREMIND_RANDOMFACILITY_TRY(
            return fromWrapper(*facility).createRandomEngine(*conf);)
}

extern "C"
SharemindRandomEngine * SharemindRandomFacility_createRandomEngineWithSeed(
        SharemindRandomFacility * facility,
        SharemindRandomEngineConf const * conf,
        const void * seedData,
        size_t seedSize,
        SharemindRandomEngineCtorError * errorPtr) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
SharemindRandomEngine * SharemindRandomFacility_createRandomEngineWithSeed(
        SharemindRandomFacility * facility,
        SharemindRandomEngineConf const * conf,
        const void * seedData,
        size_t seedSize,
        SharemindRandomEngineCtorError * errorPtr) noexcept
{
    assert(facility);

    SHAREMIND_RANDOMFACILITY_TRY(
            return fromWrapper(*facility).createRandomEngineWithSeed(*conf,
                                                                     seedData,
                                                                     seedSize);)
}

template <typename Set>
SharemindRandomEngine * wrapEngine(Set & set, RandomEngine * const engine) {
    try {
        ScopedEngine * const scopedEngine = new ScopedEngine{engine};
        try {
            set.insert(scopedEngine);
            return scopedEngine;
        } catch (...) {
            delete scopedEngine;
            throw;
        }
    } catch (...) {
        delete engine;
        throw;
    }
}

} // anonymous namespace

RandomFacility::RandomFacility(
        RandomEngineFactory::Configuration const & defaultFactoryConf)
    : SharemindRandomFacility{
          &SharemindRandomFacility_defaultFactoryConfiguration,
          &SharemindRandomFacility_createRandomEngine,
          &SharemindRandomFacility_createRandomEngineWithSeed}
    , m_engineFactory{defaultFactoryConf}
{}

RandomFacility::~RandomFacility() noexcept {
    for (auto * const engine : m_scopedEngines)
        delete static_cast<ScopedEngine *>(engine);
}

SharemindRandomEngine * RandomFacility::createRandomEngine()
{ return wrapEngine(m_scopedEngines, m_engineFactory.createRandomEngine()); }

SharemindRandomEngine * RandomFacility::createRandomEngine(
        SharemindRandomEngineConf const & conf)
{ return wrapEngine(m_scopedEngines, m_engineFactory.createRandomEngine(conf));}

SharemindRandomEngine * RandomFacility::createRandomEngineWithSeed(
        const void * const seedData,
        size_t const seedSize)
{
    return wrapEngine(m_scopedEngines,
                      m_engineFactory.createRandomEngineWithSeed(seedData,
                                                                 seedSize));
}

SharemindRandomEngine * RandomFacility::createRandomEngineWithSeed(
        SharemindRandomEngineConf const & conf,
        const void * seedData,
        size_t seedSize)
{
    return wrapEngine(m_scopedEngines,
                      m_engineFactory.createRandomEngineWithSeed(conf,
                                                                 seedData,
                                                                 seedSize));
}


} // namespace sharemind {
