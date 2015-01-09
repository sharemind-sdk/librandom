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

#ifndef SHAREMINDCOMMON_SNOW2RANDOMENGINE_H
#define SHAREMINDCOMMON_SNOW2RANDOMENGINE_H

#include "IRandom.h"

#include <cstdint>


namespace sharemind {

/**
 * \brief A randomness engine based on the SNOW 2 stream cipher.
 *
 * Initial seed is provided by the OpenSSL random engine.
 */
class Snow2RandomEngine: public IRandom {

public: /* Methods: */

    Snow2RandomEngine() noexcept
        : keystream_ready(sizeof(keystream)) {}

    void Seed() noexcept final override;

    void Seed(const void * memptr_, size_t size) noexcept final override;

    void fillBytes(void * memptr_, size_t size) noexcept final override;

private: /* Methods: */

    void snow_loadkey_fast_p(uint8_t * key,
                             uint32_t keysize,
                             uint32_t IV3,
                             uint32_t IV2,
                             uint32_t IV1,
                             uint32_t IV0);

    void snow_keystream_fast_p();

private: /* Fields: */

    uint32_t s15, s14, s13, s12, s11, s10, s9, s8, s7, s6, s5, s4, s3, s2, s1, s0;
    uint32_t r1, r2;
    uint8_t keystream_ready;
    union {
        uint32_t keystream[16];
        uint8_t un_byte_keystream[sizeof(uint32_t) * 16];
    };

}; /* class Snow2RandomEngine { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_SNOW2RANDOMENGINE_H */
