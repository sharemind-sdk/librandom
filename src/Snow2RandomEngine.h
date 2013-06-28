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

#include "Random.h"

#include <stdint.h>

namespace sharemind {

/**
 * A randomness engine based on the SNOW 2 stream cipher.
 *
 * Initial seed is provided by the OpenSSL random engine.
 */
class Snow2RandomEngine : public IRandom {
public: /* Methods: */

    Snow2RandomEngine ()
        : keystream_ready (0)
    { }

    virtual void Seed ();

    virtual void Seed (const void* memptr_, size_t size);

    virtual void fillBytes (void* memptr_, size_t size);

private: /* Fields: */

    uint8_t snowkey[32];
    uint8_t keystream_ready;
    union {
        uint32_t keystream[16];
        uint8_t  un_byte_keystream[sizeof (uint32_t) * 16];
    };

}; /* class Snow2RandomEngine { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_SNOW2RANDOMENGINE_H */
