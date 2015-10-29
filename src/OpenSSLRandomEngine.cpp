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

#include "RandomEngine.h"

#include <cassert>
#include <climits>
#include <openssl/rand.h>
#ifdef SHAREMIND_INSTRUCT_VALGRIND
#include <valgrind/memcheck.h>
#endif

namespace /* anonymous */ {

extern "C"
SharemindRandomEngineSeedError OpenSSLRandomEngine_seed_hardware(SharemindRandomEngine*) {
    return SHAREMIND_RANDOM_SEED_OK;
}

extern "C"
SharemindRandomEngineSeedError OpenSSLRandomEngine_seed(SharemindRandomEngine*, const void *, size_t) {
    return SHAREMIND_RANDOM_SEED_NOT_SUPPORTED;
}

extern "C"
void OpenSSLRandomEngine_fill_bytes(SharemindRandomEngine*, void* memptr_, size_t size) {
    assert (size <= INT_MAX);
    const auto memptr = static_cast<unsigned char*>(memptr_);
    #ifndef NDEBUG
    auto const r =
    #endif
        RAND_bytes(memptr, size);
    assert(r == 1);
}

extern "C"
void OpenSSLRandomEngine_free(SharemindRandomEngine*) { }

static SharemindRandomEngine OpenSSL_random_engine = SharemindRandomEngine {
    size_t(0),
    OpenSSLRandomEngine_seed_hardware,
    OpenSSLRandomEngine_seed,
    OpenSSLRandomEngine_fill_bytes,
    OpenSSLRandomEngine_free
};

} // namespace anonymous

namespace sharemind {

SharemindRandomEngine* make_OpenSSL_random_engine() noexcept {
    return &OpenSSL_random_engine;
}

} // namespace sharemind
