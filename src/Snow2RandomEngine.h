/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_SNOW2RANDOMENGINE_H
#define SHAREMINDCOMMON_SNOW2RANDOMENGINE_H

#include "IRandom.h"

#if __cplusplus >= 201103L
#include <cstdint>
#else
#include <stdint.h>
#endif


namespace sharemind {

/**
 * \brief A randomness engine based on the SNOW 2 stream cipher.
 *
 * Initial seed is provided by the OpenSSL random engine.
 */
class Snow2RandomEngine: public IRandom {

public: /* Methods: */

    Snow2RandomEngine()
        : keystream_ready(sizeof(keystream)) {}

    virtual void Seed();

    virtual void Seed(const void * memptr_, size_t size);

    virtual void fillBytes(void * memptr_, size_t size);

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
