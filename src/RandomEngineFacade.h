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

#ifndef SHAREMIND_LIBRANDOM_RANDOMENGINEFACADE_H
#define SHAREMIND_LIBRANDOM_RANDOMENGINEFACADE_H

#include "librandom.h"

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <vector>
#include <type_traits>


namespace sharemind {

/**
 * \brief Facade for \a SharemindRandomEngine.
 */
class RandomEngineFacade {

public: /* Methods: */

    inline explicit RandomEngineFacade (SharemindRandomEngine * rng)
        : m_inner (rng)
    { assert (m_inner != nullptr); }

    RandomEngineFacade (const RandomEngineFacade &) = delete;
    RandomEngineFacade & operator = (const RandomEngineFacade &) = delete;

    RandomEngineFacade (RandomEngineFacade && other)
        : m_inner (other.m_inner)
    { other.m_inner = nullptr; }

    RandomEngineFacade & operator = (RandomEngineFacade && other) {
        if (this != &other) {
            m_inner = other.m_inner;
            other.m_inner = nullptr;
        }

        return *this;
    }

    inline void fillBytes (void* memptr, size_t numBytes) noexcept {
        assert (m_inner != nullptr);
        m_inner->fillBytes (m_inner, memptr, numBytes);
    }

    template <typename T>
    inline void fillBlock(T * begin, T * end) noexcept {
        assert (m_inner != nullptr);
        assert(begin <= end);
        if (begin < end) {
            auto const dist = std::distance(begin, end);
            using U = typename std::make_unsigned<decltype(dist)>::type;
            fillBytes(begin, sizeof(T) * static_cast<U>(dist));
        }
    }

    template <typename T>
    inline T randomValue() noexcept(noexcept(T(T()))) {
        assert (m_inner != nullptr);
        T value;
        fillBytes(&value, sizeof(T));
        return value;
    }

private: /* Fields: */
    SharemindRandomEngine * m_inner;

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_RANDOMENGINEFACADE_H */
