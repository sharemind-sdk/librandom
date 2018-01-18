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

#ifndef SHAREMIND_LIBRANDOM_RANDOMENGINEFACTORY_H
#define SHAREMIND_LIBRANDOM_RANDOMENGINEFACTORY_H

#include "librandom.h"

#include <cstdlib>
#include <memory>
#include <sharemind/Exception.h>
#include <sharemind/ExceptionMacros.h>


namespace sharemind {

class RandomEngine;

class RandomEngineFactory {

public: /* Types: */

    using Configuration = SharemindRandomEngineConf;

    SHAREMIND_DEFINE_EXCEPTION(sharemind::Exception, Exception);
    SHAREMIND_DEFINE_EXCEPTION(Exception, RandomCtorSeedException);

    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(Exception,
                                         RandomCtorGeneratorNotSupported,
                                         "Unsupported generator");

    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(Exception,
                                         RandomCtorOtherError,
                                         "Failed to construct the generator");

    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorSeedException,
                                         RandomCtorSeedTooShort,
                                         "Provided seed is too short");

    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(RandomCtorSeedException,
                                         RandomCtorSeedSelfGenerateError,
                                         "Failed to self-generate a seed");

    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(
            RandomCtorSeedException,
            RandomCtorSeedNotSupported,
            "Providing a fixed seed is not supported by this generator");

public: /* Methods: */

    inline RandomEngineFactory(Configuration const & defaultConf)
        : m_defaultConf(defaultConf)
    {}

    inline Configuration const & defaultConfiguration() const noexcept
    { return m_defaultConf; }

    inline size_t getSeedSize() const noexcept
    { return getSeedSize(m_defaultConf.coreEngine); }

    static size_t getSeedSize(SharemindCoreRandomEngineKind kind) noexcept;

    inline std::shared_ptr<RandomEngine> createRandomEngineWithSeed(
            const void * const seedData,
            size_t const seedSize) const
    { return createRandomEngineWithSeed(m_defaultConf, seedData, seedSize); }

    static std::shared_ptr<RandomEngine> createRandomEngineWithSeed(
            Configuration const & conf,
            const void * seedData,
            size_t seedSize);

private: /* Fields: */

    Configuration const m_defaultConf;

}; /* class RandomEngineFactory { */

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_RANDOMENGINEFACTORY_H */
