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

namespace sharemind {

/**
 * \brief Facade for the \a SharemindRandomEngineFactory structure.
 */
class RandomEngineFactory {
public: /* Methods: */

    inline explicit RandomEngineFactory (SharemindRandomEngineFactoryFacility* factory)
        : m_inner (factory)
    { }

    ~RandomEngineFactory() {
        if (m_inner != nullptr) {
            m_inner->free(m_inner);
        }
    }

    RandomEngineFactory (const RandomEngineFactory&) = delete;
    RandomEngineFactory& operator = (const RandomEngineFactory&) = delete;

    RandomEngineFactory (RandomEngineFactory&& other)
        : m_inner (other.m_inner)
    { other.m_inner = nullptr; }

    RandomEngineFactory& operator = (RandomEngineFactory&& other) {
        if (this != &other) {
            m_inner = other.m_inner;
            other.m_inner = nullptr;
        }

        return *this;
    }

    inline SharemindRandomEngineConf getDefaultConfiguration() const noexcept {
        assert (m_inner != nullptr);
        return m_inner->get_default_configuration(m_inner);
    }

    inline SharemindRandomEngine* getRandomEngine() const noexcept {
        assert (m_inner != nullptr);
        return m_inner->get_random_engine(m_inner, getDefaultConfiguration());
    }

    inline SharemindRandomEngine* getRandomEngine(SharemindRandomEngineConf conf) const noexcept {
        assert (m_inner != nullptr);
        return m_inner->get_random_engine(m_inner, conf);
    }

    inline SharemindRandomEngineFactoryFacility* getSharemindRandomEngineFactoryFacility() noexcept {
        return m_inner;
    }

    inline const SharemindRandomEngineFactoryFacility* getSharemindRandomEngineFactoryFacility() const noexcept {
        return m_inner;
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
