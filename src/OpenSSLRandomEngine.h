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

#ifndef SHAREMINDCOMMON_OPENSSLRANDOMENGINE_H
#define SHAREMINDCOMMON_OPENSSLRANDOMENGINE_H

#include "IRandom.h"


namespace sharemind {

/** A randomness engine based on OpenSSL RAND_bytes(). */
class OpenSSLRandomEngine: public IRandom {

public: /* Methods: */

    inline void Seed() noexcept final override {}

    void Seed(const void * memptr, size_t size) noexcept final override;

    void fillBytes(void * memptr, size_t size) noexcept final override;

};

} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_OPENSSLRANDOMENGINE_H */
