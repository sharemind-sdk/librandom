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

#ifndef SHAREMIND_LIBRANDOME_RANDOMENGINE_H
#define SHAREMIND_LIBRANDOME_RANDOMENGINE_H

#include "librandom.h"

#include <cstdlib>

namespace sharemind {

/**
 * \brief Facade for the \a SharemindRandomEngine structure.
 */
class RandomEngine {
public: /* Methods: */

    inline explicit RandomEngine (SharemindRandomEngine* rng)
        : m_inner (rng)
    { }

    RandomEngine (const RandomEngine&) = delete;
    RandomEngine& operator = (const RandomEngine&) = delete;

    RandomEngine (RandomEngine&& other)
        : m_inner (other.m_inner)
    { other.m_inner = nullptr; }

    RandomEngine& operator = (RandomEngine&& other) {
        if (this != &other) {
            m_inner = other.m_inner;
            other.m_inner = nullptr;
        }

        return *this;
    }

    inline ~RandomEngine () {
        if (m_inner) {
            m_inner->free (m_inner);
        }
    }

    inline size_t seedSize () const noexcept {
        return m_inner->seed_size;
    }

    inline void seedHardware () {
        switch (m_inner->seed_hardware (m_inner)) {
        case SHAREMIND_RANDOM_SEED_OK:
            break;
        default:
            abort ();
            break;
        }
    }

    inline void seed (const void* memptr, size_t numBytes) {
        switch (m_inner->seed (m_inner, memptr, numBytes)) {
        case SHAREMIND_RANDOM_SEED_OK:
            break;
        default:
            abort ();
            break;
        }
    }

    inline void fillBytes (void* memptr, size_t numBytes) noexcept {
        m_inner->fill_bytes (m_inner, memptr, numBytes);
    }

    inline SharemindRandomEngine* getSharemindRandomEngine() noexcept { return m_inner; }
    inline const SharemindRandomEngine* getSharemindRandomEngine() const noexcept { return m_inner; }

private: /* Fields: */
    SharemindRandomEngine* m_inner;
};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOME_RANDOMENGINE_H */
