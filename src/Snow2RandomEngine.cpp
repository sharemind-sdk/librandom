/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include "Snow2RandomEngine.h"

#include <cstring>
#include "OpenSSLRandomEngine.h"
extern "C" {
#include "snow2_fast.h"
}


namespace sharemind {

void Snow2RandomEngine::Seed () {
    uint8_t snowkey[32];
    uint32_t iv [4];
    OpenSSLRandomEngine rng;
    rng.fillBytes (snowkey, sizeof (snowkey));
    rng.fillBytes (iv, sizeof (iv));
    snow_loadkey_fast (snowkey, 128, iv[0], iv[1], iv[2], iv[3]);
}

void Snow2RandomEngine::Seed (const void* memptr_, size_t size) {
    uint8_t snowkey[32];
    uint32_t iv [4];
    assert (size == sizeof (snowkey) + sizeof (iv));
    if (size != sizeof (snowkey) + sizeof (iv)) {
        // Fallback in case of misuse in non-debug situation.
        Seed ();
    }
    else {
        const uint8_t* memptr = static_cast<const uint8_t*>(memptr_);
        memcpy (snowkey, memptr, sizeof (snowkey));
        memcpy (iv, memptr + sizeof (snowkey), sizeof (iv));
        snow_loadkey_fast (snowkey, 128, iv[0], iv[1], iv[2], iv[3]);
    }
}

void Snow2RandomEngine::fillBytes (void* memptr_, size_t size) {
    uint8_t* memptr = static_cast<uint8_t*>(memptr_);
    size_t currentKeystreamSize = sizeof (keystream) - keystream_ready;
    size_t offsetStart = 0;
    size_t offsetEnd = currentKeystreamSize;

    // Fill big chunks
    while (offsetEnd <= size) {
        memcpy (memptr + offsetStart, &un_byte_keystream[keystream_ready], currentKeystreamSize);
        snow_keystream_fast(keystream);
        keystream_ready = 0;
        currentKeystreamSize = sizeof (keystream);
        offsetStart = offsetEnd;
        offsetEnd += sizeof (keystream);
    }

    const size_t remainingSize = size - offsetStart;
    memcpy (memptr + offsetStart, &un_byte_keystream[keystream_ready], remainingSize);
    keystream_ready += remainingSize;
    assert (keystream_ready <= sizeof(keystream)); // the supply may deplete
    return;
}

} // namespace sharemind
