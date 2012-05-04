/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "OpenSSLRandomEngine.h"

#include <assert.h>
#include <openssl/rand.h>


namespace sharemind {

void OpenSSLRandomEngine::Seed () {
    // Possibly incorporate more entropy
}

void OpenSSLRandomEngine::fillBytes (void* memptr, size_t size) {
    const bool RANDbytesOK = (RAND_bytes (static_cast<unsigned char*>(memptr), size) == 1);
    assert (RANDbytesOK);
}

} // namespace sharemind
