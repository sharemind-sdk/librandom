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

#ifndef SHAREMIND_LIBRANDOM_SNOW2RANDOMENGINE_H
#define SHAREMIND_LIBRANDOM_SNOW2RANDOMENGINE_H

#include "RandomEngine.h"

#include <array>
#include <cstdint>


namespace sharemind {

class Snow2RandomEngine: public RandomEngine {

public: /* Types: */

    constexpr static size_t const SeedSize = 48u;

public: /* Methods: */

    explicit Snow2RandomEngine(const void * const seed);

    void fillBytes(void * buffer, size_t size) noexcept override;

private: /* Fields: */

    std::array<uint32_t, 16u> s;
    uint32_t r1, r2;
    union {
        std::array<uint32_t, 16u> keystream;
        std::array<uint8_t, sizeof(uint32_t) * 16> un_byte_keystream;
    };
    static_assert(sizeof(keystream) == 64u, "");
    static_assert(sizeof(un_byte_keystream) == 64u, "");
    unsigned haveData = 0u;

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_SNOW2RANDOMENGINE_H */
