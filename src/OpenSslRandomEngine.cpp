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

#include "OpenSslRandomEngine.h"

#include <cassert>
#include <limits>
#include <openssl/rand.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>

namespace sharemind {

void OpenSslRandomEngine::staticFillBytes(void * buffer,
                                          size_t bufferSize) noexcept
{
    static constexpr int const MAX = std::numeric_limits<int>::max();
    static auto const doFill = [](void * const buf, size_t const size) {
        #ifndef NDEBUG
        auto const r =
        #endif
            RAND_bytes(static_cast<unsigned char *>(buf),
                       static_cast<int>(size));
        assert(r == 1);
    };
    while (bufferSize > MAX) {
        doFill(buffer, MAX);
        bufferSize -= MAX;
        buffer = ptrAdd(buffer, MAX);
    }
    doFill(buffer, bufferSize);
}

} // namespace sharemind
