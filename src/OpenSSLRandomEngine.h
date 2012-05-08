/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_OPENSSLRANDOMENGINE_H
#define SHAREMINDCOMMON_OPENSSLRANDOMENGINE_H

#include "Random.h"

namespace sharemind {

/** A randomness engine based on OpenSSL RAND_bytes(). */
class OpenSSLRandomEngine : public IRandom {
public: /* Methods: */

    /** Constructs the generator and seeds it. */
    virtual void Seed();

    virtual void fillBytes (void* memptr, size_t size);
};

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_OPENSSLRANDOMENGINE_H */
