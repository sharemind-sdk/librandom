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

#ifndef SHAREMIND_LIBRANDOM_RANDOMENGINE_H
#define SHAREMIND_LIBRANDOM_RANDOMENGINE_H

#include "librandom.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <sharemind/Exception.h>

namespace sharemind {

class RandomEngine {

public: /* Types: */

    SHAREMIND_DEFINE_EXCEPTION(std::exception, Exception);
    SHAREMIND_DEFINE_EXCEPTION_CONST_MSG(Exception,
                                         GeneratorNotSupportedException,
                                         "Generator not supported!");

public: /* Methods: */

    virtual ~RandomEngine() noexcept;

    virtual void fillBytes(void * buffer, size_t size) noexcept = 0;

    template <typename T>
    inline void fillBlock(T * begin, T * end) noexcept {
        assert(begin <= end);
        if (begin < end)
            fillBytes(begin, sizeof(T) * (end - begin));
    }

    template <typename T>
    inline T randomValue() noexcept(noexcept(T(T()))) {
        T value;
        fillBytes(&value, sizeof(T));
        return value;
    }

};

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_RANDOMENGINE_H */
