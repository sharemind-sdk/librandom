/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
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
    const bool RANDbytesOK =
    #endif
            (RAND_bytes(static_cast<unsigned char*>(memptr), size) == 1);
    assert (RANDbytesOK);
}

} // namespace sharemind
