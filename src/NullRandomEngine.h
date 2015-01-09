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
