/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_IRANDOM_H
#define SHAREMINDCOMMON_IRANDOM_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fluffy/Exception.h>


namespace sharemind {

/** This class is a header-only front-end for randomness engines. */
class IRandom {

public: /* Types: */

    FLUFFY_DEFINE_EXCEPTION_UNUSED(Fluffy::Exception, Exception);

public: /* Methods: */

    virtual ~IRandom() noexcept {}

    virtual void Seed() noexcept = 0;

    virtual void Seed(const void * memptr, size_t size) noexcept = 0;

    /**
     * \brief Fills memory with randomly generated bytes.
     * \param[in] memptr pointer to memory to fill
     * \param[in] size number of bytes to fill
     */
    virtual void fillBytes(void * memptr, size_t size) noexcept = 0;

    template <typename T>
    inline void fillBlock(T * begin, T * end) noexcept {
        assert(begin <= end);
        if (begin < end)
            fillBytes(begin, sizeof(T)*std::distance(begin, end));
    }

    inline size_t operator()(void * memptr, size_t size) noexcept {
        fillBytes(memptr, size);
        return size;
    }

}; /* class IRandom { */

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_IRANDOM_H */
