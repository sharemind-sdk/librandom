/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_NULLRANDOMENGINE_H
#define SHAREMINDCOMMON_NULLRANDOMENGINE_H

#include "IRandom.h"


namespace sharemind {

/** A randomness engine that only generates zeroes. Useful for debugging. */
class NullRandomEngine: public IRandom {

public: /* Methods: */

    inline void Seed() noexcept final override {}

    inline void Seed(const void * memptr, size_t size) noexcept final override
    { (void) memptr; (void) size; }

    void fillBytes(void * memptr, size_t size) noexcept final override;

};

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_NULLRANDOMENGINE_H */
