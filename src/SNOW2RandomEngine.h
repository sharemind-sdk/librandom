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

#include "librandom.h"

#include <array>
#include <cstdint>


namespace sharemind {

using Snow2Key = std::array<uint8_t, 32u>;
using Snow2Iv = std::array<uint32_t, 4u>;

class SNOW2RandomEngine: public SharemindRandomEngine {

private: /* Types: */

    template <typename Array>
    struct SizeOfArrayInBytes {
        static constexpr size_t value = std::tuple_size<Array>::value
                                        * sizeof(typename Array::value_type);
        static_assert(sizeof(Array) == value, "");
    };

public: /* Types: */

    constexpr static size_t const SeedSize =
            SizeOfArrayInBytes<Snow2Key>::value
            + SizeOfArrayInBytes<Snow2Iv>::value;

public: /* Methods: */

    /** \param[in] memptr pointer to the seed of SeedSize bytes. */
    explicit SNOW2RandomEngine(const void * const memptr);

    virtual ~SNOW2RandomEngine() noexcept;

    void fillBytes(void * memptr, size_t size) noexcept;

    inline static SNOW2RandomEngine & fromWrapper(SharemindRandomEngine & base)
            noexcept
    { return static_cast<SNOW2RandomEngine &>(base); }

    inline static SNOW2RandomEngine const & fromWrapper(
            SharemindRandomEngine const & base) noexcept
    { return static_cast<SNOW2RandomEngine const &>(base); }

private: /* Fields: */

    void * const m_inner;

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_SNOW2RANDOMENGINE_H */
