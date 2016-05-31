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
#include "CryptographicRandom.h"
#include "RandomEngine.h"


namespace sharemind {

namespace {

extern "C" void SharemindRandomEngine_fillBytes(SharemindRandomEngine * rng,
                                                void * memptr,
                                                size_t size) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

inline RandomFacility::ScopedEngine & fromWrapper(SharemindRandomEngine & base)
        noexcept
{ return static_cast<RandomFacility::ScopedEngine &>(base); }

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
void SharemindRandomFacility_RandomBlocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
void SharemindRandomFacility_RandomBlocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
{
    assert(facility);
    fromWrapper(*facility).RandomBlocking(memptr, size);
}

extern "C"
void SharemindRandomFacility_URandomBlocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
void SharemindRandomFacility_URandomBlocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
{
    assert(facility);
    fromWrapper(*facility).URandomBlocking(memptr, size);
}

extern "C"
size_t SharemindRandomFacility_RandomNonblocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
size_t SharemindRandomFacility_RandomNonblocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
{
    assert(facility);
    return fromWrapper(*facility).RandomNonblocking(memptr, size);
}

extern "C"
size_t SharemindRandomFacility_URandomNonblocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
size_t SharemindRandomFacility_URandomNonblocking(
        SharemindRandomFacility * facility,
        void * memptr,
        size_t size) noexcept
{
    assert(facility);
    return fromWrapper(*facility).RandomNonblocking(memptr, size);
}

extern "C"
size_t SharemindRandomFacility_getSeedSize(
        SharemindRandomFacility * facility,
        SharemindRandomEngineConf const * conf) noexcept
        SHAREMIND_VISIBILITY_HIDDEN;

extern "C"
size_t SharemindRandomFacility_getSeedSize(
        SharemindRandomFacility * facility,
        SharemindRandomEngineConf const * conf) noexcept
{
    assert(facility);
    return fromWrapper(*facility).getSeedSize(*conf);
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
            return fromWrapper(*facility).createRandomEngineWithSeed(
                            *conf,
                            seedData,
                            seedSize).get();)
}

} // anonymous namespace


RandomFacility::ScopedEngine::ScopedEngine(std::shared_ptr<RandomEngine> engine)
    : SharemindRandomEngine{&SharemindRandomEngine_fillBytes}
    , m_engine{std::move((assert(engine), engine))}
{}

RandomFacility::RandomFacility(
        RandomEngineFactory::Configuration const & defaultFactoryConf)
    : SharemindRandomFacility{
          {
            &SharemindRandomFacility_RandomBlocking,
            &SharemindRandomFacility_URandomBlocking,
            &SharemindRandomFacility_RandomNonblocking,
            &SharemindRandomFacility_URandomNonblocking,
          },
          &SharemindRandomFacility_defaultFactoryConfiguration,
          &SharemindRandomFacility_getSeedSize,
          &SharemindRandomFacility_createRandomEngineWithSeed}
    , m_engineFactory{defaultFactoryConf}
{}

void RandomFacility::RandomBlocking(void * memptr, size_t size) const noexcept
{ sharemindCyptographicRandom(memptr, size); }

void RandomFacility::URandomBlocking(void * memptr, size_t size) const noexcept
{ sharemindCyptographicURandom(memptr, size); }

size_t RandomFacility::RandomNonblocking(void * memptr, size_t size) const noexcept
{ return sharemindCryptographicRandomNonblocking(memptr, size); }

size_t RandomFacility::URandomNonblocking(void * memptr, size_t size) const noexcept
{ return sharemindCryptographicURandomNonblocking(memptr, size); }

size_t RandomFacility::getSeedSize(SharemindRandomEngineConf const & conf) const
{ return RandomEngineFactory::getSeedSize(conf.coreEngine); }

std::shared_ptr<SharemindRandomEngine>
RandomFacility::createRandomEngineWithSeed(
        SharemindRandomEngineConf const & conf,
        const void * seedData,
        size_t seedSize)
{
    auto scopedEngine(
                std::make_shared<RandomFacility::ScopedEngine>(
                    m_engineFactory.createRandomEngineWithSeed(conf,
                                                               seedData,
                                                               seedSize)));
    m_scopedEngines.emplace_back(scopedEngine);
    return scopedEngine;
}


} // namespace sharemind {
