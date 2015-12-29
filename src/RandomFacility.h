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

#ifndef SHAREMIND_LIBRANDOM_RANDOMFACILITY_H
#define SHAREMIND_LIBRANDOM_RANDOMFACILITY_H

#include "librandom.h"

#include <set>
#include "RandomEngineFactory.h"


namespace sharemind {

class RandomFacility: public SharemindRandomFacility {

public: /* Methods: */

    RandomFacility(
            RandomEngineFactory::Configuration const & defaultFactoryConf);

    virtual ~RandomFacility() noexcept;

    SharemindRandomFacility & facility() noexcept
    { return static_cast<SharemindRandomFacility &>(*this); }

    inline SharemindRandomEngineConf const & defaultFactoryConfiguration()
            const noexcept
    { return m_engineFactory.defaultConfiguration(); }

    SharemindRandomEngine * createRandomEngine();

    SharemindRandomEngine * createRandomEngine(
            SharemindRandomEngineConf const & conf);

    SharemindRandomEngine * createRandomEngineWithSeed(
            const void * const seedData,
            size_t const seedSize);

    SharemindRandomEngine * createRandomEngineWithSeed(
            SharemindRandomEngineConf const & conf,
            const void * seedData,
            size_t seedSize);

private: /* Fields: */

    RandomEngineFactory m_engineFactory;
    std::set<void *> m_scopedEngines;

};


} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_RANDOMFACILITY_H */
