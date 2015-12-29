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

#ifndef SHAREMIND_LIBRANDOM_NULLRANDOMENGINE_H
#define SHAREMIND_LIBRANDOM_NULLRANDOMENGINE_H

#include "RandomEngine.h"
#include <cstring>


namespace sharemind {

class NullRandomEngine: public RandomEngine {

public: /* Methods: */

    inline void fillBytes(void * memptr, size_t numBytes) noexcept
    { memset(memptr, 0, numBytes); }

    static inline NullRandomEngine & instance() noexcept;

};

inline NullRandomEngine & NullRandomEngine::instance() noexcept {
    static NullRandomEngine instance;
    return instance;
}

} /* namespace sharemind { */

#endif /* SHAREMIND_LIBRANDOM_NULLRANDOMENGINE_H */
