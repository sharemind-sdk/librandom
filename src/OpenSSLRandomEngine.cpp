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

#include "OpenSSLRandomEngine.h"

#include <cassert>
#include <openssl/rand.h>
#ifdef SHAREMIND_INSTRUCT_VALGRIND
#include <valgrind/memcheck.h>
#endif


namespace sharemind {

void OpenSSLRandomEngine::Seed(const void * memptr, size_t size) noexcept {
     RAND_seed (memptr, size);
}

void OpenSSLRandomEngine::fillBytes (void * memptr, size_t size) noexcept {
    #ifdef SHAREMIND_INSTRUCT_VALGRIND
    VALGRIND_MAKE_MEM_DEFINED(memptr, size);
    #endif
    #ifndef NDEBUG
    auto const r =
    #endif
            RAND_bytes(static_cast<unsigned char*>(memptr), size);
    assert(r == 1);
}

} // namespace sharemind
